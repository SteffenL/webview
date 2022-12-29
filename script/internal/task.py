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
    status_queue: queue.Queue
    lock: threading.Lock
    task_count: int
    tasks_done: int
    next_task_number: int

    def get_next_task_number(self):
        n = self.next_task_number
        self.next_task_number += 1
        return n

    def one_done(self):
        self.tasks_done += 1


@dataclass
class TaskWorkPerCollection:
    task_count: int
    tasks_done: int

    def one_done(self):
        self.tasks_done += 1


class TaskStatus(Enum):
    STARTED = 0
    DONE = 1


class TaskRunner:
    _collections: List[TaskCollection]

    def __init__(self):
        self._collections = []

    def execute(self, on_status: Callable[[TaskStatus, str, bool], None] = lambda *_: None):
        shared = TaskWorkShared(
            status_queue=queue.Queue(),
            lock=threading.Lock(),
            task_count=self.get_task_count(),
            tasks_done=0,
            next_task_number=1
        )

        def task_wrapper(shared: TaskWorkShared, per_collection: TaskWorkPerCollection, task: Task):
            with LockScope(shared.lock):
                task_number = shared.get_next_task_number()
            shared.status_queue.put((task_number, task, TaskStatus.STARTED))
            task.execute()
            shared.status_queue.put((task_number, task, TaskStatus.DONE))
            with LockScope(shared.lock):
                shared.one_done()
                per_collection.one_done()

        for collection in self._collections:
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
                    executor.submit(task_wrapper, shared, per_collection, task)
                while True:
                    with LockScope(shared.lock):
                        done = per_collection.tasks_done >= per_collection.task_count
                    if done and shared.status_queue.empty():
                        break
                    task_number, task, status = shared.status_queue.get()
                    on_status(status, task.get_description(), is_concurrent)

    def get_task_count(self):
        return sum(c.get_count() for c in self._collections)

    def create_task_collection(self, concurrent: bool = False):
        collection = TaskCollection(concurrent=concurrent)
        self.add_task_collection(collection)
        return collection

    def add_task_collection(self, *collections: TaskCollection):
        self._collections += collections
