from concurrent.futures import Future, ThreadPoolExecutor
from typing import Any, Callable, Generic, List, TypeVar
import uuid


T = TypeVar("T")


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


class TaskRunner:
    _collections: List[TaskCollection]

    def __init__(self):
        self._collections = []

    def execute(self, on_status: Callable[[int, str], None] = lambda *_: None):
        task_count = self.get_task_count()
        task_number = 0
        for collection in self._collections:
            concurrent = collection.is_concurrent()
            workers = None if concurrent else 1
            futures: list[Future] = []
            with ThreadPoolExecutor(max_workers=workers) as executor:
                for task in collection.get_tasks():
                    task_number += 1
                    on_status(task_number, task_count, task.get_description())
                    future = executor.submit(task.execute)
                    futures.append(future)
                    if not concurrent:
                        future.result()
                if concurrent:
                    for future in futures:
                        future.result()

    def get_task_count(self):
        return sum(c.get_count() for c in self._collections)

    def create_task_collection(self, concurrent: bool = False):
        collection = TaskCollection(concurrent=concurrent)
        self.add_task_collection(collection)
        return collection

    def add_task_collection(self, *collections: TaskCollection):
        self._collections += collections
