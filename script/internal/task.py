from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from enum import Enum
import queue
import threading
from typing import Any, Callable, Generic, List, TypeVar
import uuid


T = TypeVar("T")


class LockScope:
    _lock: threading.Lock

    def __init__(self, lock: threading.Lock) -> None:
        self._lock = lock

    def __enter__(self):
        self._lock.acquire()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._lock.release()


class Task(Generic[T]):
    _id: str
    _work: Callable[[T], None]
    _arg: T
    _description: str
    _condition: Callable[[T], bool]

    def __init__(self, work: Callable[[T], None], arg: T = None, description: str = None, condition: Callable[[T], bool] = lambda *_: True):
        self._id = uuid.uuid4().hex
        self._work = work
        self._arg = arg
        self._description = description
        self._condition = condition

    def execute(self) -> Any:
        if self._work:
            self._work(self._arg)

    def get_id(self):
        return self._id

    def get_description(self):
        return self._description

    def is_condition_met(self):
        return self._condition(self._arg)


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
    STARTED = 0
    DONE = 1
    FAILED = 2


class TaskRunner:
    _collections: List[TaskCollection]

    def __init__(self):
        self._collections = []

    def execute(self, on_status: Callable[[TaskStatus, str, bool], None] = lambda *_: None):
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
                    with LockScope(shared.lock):
                        done = per_collection.tasks_done >= per_collection.task_count
                    if done and shared.status_queue.empty():
                        break
                    task, status, exception = shared.status_queue.get()
                    on_status(status, task.get_description(), is_concurrent)
                    if status == TaskStatus.FAILED:
                        force_stop = True
                        shared.stop = True
                if force_stop:
                    executor.shutdown(cancel_futures=True)
            if status == TaskStatus.FAILED:
                raise Exception("One or more tasks failed.")

    @staticmethod
    def _worker(shared: TaskWorkShared, per_collection: TaskWorkPerCollection, task: Task):
        with LockScope(shared.lock):
            if shared.stop:
                return
        shared.status_queue.put((task, TaskStatus.STARTED, None))
        try:
            task.execute()
        except Exception as e:
            with LockScope(shared.lock):
                shared.stop = True
            shared.status_queue.put((task, TaskStatus.FAILED, e))
            return
        shared.status_queue.put((task, TaskStatus.DONE, None))
        with LockScope(shared.lock):
            per_collection.one_done()

    def get_task_count(self):
        return sum(c.get_count() for c in self._collections)

    def create_task_collection(self, concurrent: bool = False):
        collection = TaskCollection(concurrent=concurrent)
        self.add_task_collection(collection)
        return collection

    def add_task_collection(self, *collections: TaskCollection):
        self._collections += collections
