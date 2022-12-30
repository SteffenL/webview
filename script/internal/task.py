from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from enum import Enum
import queue
import sys
import threading
from typing import Any, Callable, Generic, List, TypeVar
import uuid


T = TypeVar("T")


class Task(Generic[T]):
    _id: str
    _work: Callable[["Task", T], None]
    _arg: T
    _description: str
    _condition: Callable[[T], bool]
    _result: T
    _exception: Exception
    _lock: threading.Lock
    _status: "TaskStatus"

    def __init__(self, work: Callable[["Task", T], None], arg: T = None, description: str = None, condition: Callable[[T], bool] = lambda *_: True):
        self._id = uuid.uuid4().hex
        self._work = work
        self._arg = arg
        self._description = description
        self._condition = condition
        self._result = None
        self._exception = None
        self._lock = threading.Lock()
        self._status = TaskStatus.IDLE

    def execute(self) -> Any:
        if self._work:
            self._work(self, self._arg)

    def get_id(self):
        return self._id

    def get_description(self):
        return self._description

    def is_condition_met(self):
        return self._condition(self._arg)

    def get_result(self):
        with self._lock:
            return self._result

    def get_exception(self):
        with self._lock:
            return self._exception

    def set_result(self, value: Any, status: "TaskStatus" = None):
        with self._lock:
            self._result = value
            if status is not None:
                self.set_status(status)

    def set_exception(self, e: Exception):
        with self._lock:
            self._exception = e

    def set_status(self, status: "TaskStatus"):
        self._status = status

    def get_status(self):
        return self._status

class TaskCollection:
    _concurrent: bool
    _tasks: List[Task]

    def __init__(self, concurrent: bool = False):
        self._concurrent = concurrent
        self._tasks = []

    def is_concurrent(self):
        return self._concurrent

    def add_task(self, *task: Task[T]):
        self._tasks += task

    def get_tasks(self, all: bool = False):
        return tuple(t for t in self._tasks if all or t.is_condition_met())

    def get_count(self, all: bool = False):
        return len(self.get_tasks(all=all))


@dataclass
class TaskWorkShared:
    stop: bool
    status_queue: queue.Queue
    lock: threading.Lock


@dataclass
class TaskWorkPerCollection:
    task_count: int
    tasks_done: int

    def one_done(self):
        self.tasks_done += 1


class TaskStatus(Enum):
    IDLE = 0
    STARTED = 1
    DONE = 2
    FAILED = 3
    CANCELED = 4


class TaskRunner:
    _collections: List[TaskCollection]

    def __init__(self):
        self._collections = []

    def execute(self, on_status: Callable[[TaskStatus, str, bool, bytes, Exception], None] = lambda *_: None):
        shared = TaskWorkShared(
            stop=False,
            status_queue=queue.Queue(),
            lock=threading.Lock()
        )

        status = None
        force_stop = False
        for collection in self._collections:
            if force_stop:
                break
            tasks = collection.get_tasks()
            if len(tasks) == 0:
                continue
            is_concurrent = collection.is_concurrent()
            worker_count = None if is_concurrent else 1
            with ThreadPoolExecutor(max_workers=worker_count) as executor:
                per_collection = TaskWorkPerCollection(
                    task_count=len(tasks),
                    tasks_done=0
                )
                for task in tasks:
                    executor.submit(self._worker, shared, per_collection, task)
                while not force_stop:
                    with shared.lock:
                        done = per_collection.tasks_done >= per_collection.task_count
                    if done and shared.status_queue.empty():
                        break
                    task, status = shared.status_queue.get()
                    output = task.get_result()
                    e = task.get_exception()
                    on_status(status, task.get_description(), is_concurrent, output, e)
                    if status == TaskStatus.FAILED:
                        force_stop = True
                        with shared.lock:
                            shared.stop = True
                if force_stop:
                    executor.shutdown(cancel_futures=True)

    @staticmethod
    def _worker(shared: TaskWorkShared, per_collection: TaskWorkPerCollection, task: Task):
        with shared.lock:
            if shared.stop:
                task.set_status(TaskStatus.CANCELED)
                per_collection.one_done()
                shared.status_queue.put((task, TaskStatus.CANCELED))
                return
        task.set_status(TaskStatus.STARTED)
        shared.status_queue.put((task, TaskStatus.STARTED))
        status: TaskStatus
        try:
            task.execute()
            status = TaskStatus.DONE
        except Exception as e:
            status = TaskStatus.FAILED
            task.set_exception(e)
        task.set_status(status)
        per_collection.one_done()
        shared.status_queue.put((task, status))

    def get_task_count(self):
        return sum(c.get_count() for c in self._collections)

    def create_task_collection(self, concurrent: bool = False):
        collection = TaskCollection(concurrent=concurrent)
        self.add_task_collection(collection)
        return collection

    def add_task_collection(self, *collections: TaskCollection):
        self._collections += collections
