from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from enum import Enum
import queue
import threading
from typing import Any, Callable, Generic, List, Mapping, Sequence, Tuple, TypeVar, Union
import uuid


T = TypeVar("T")
AnyArgs = Tuple[T, ...]
WorkFunction = Callable[["Task", AnyArgs], None]
ConditionFunction = Callable[["Task", AnyArgs], bool]


class Task(Generic[T]):
    _id: str
    _work: WorkFunction
    _args: Sequence[Any]
    _description: str
    _condition: ConditionFunction
    _result: T
    _exception: Exception
    _lock: threading.Lock
    _status: "TaskStatus"

    def __init__(self,
                 work: WorkFunction,
                 args: AnyArgs = tuple(),
                 description: str = None,
                 condition: ConditionFunction = lambda *_: True):
        self._id = uuid.uuid4().hex
        self._work = work
        self._args = args
        self._description = description
        self._condition = condition
        self._result = None
        self._exception = None
        self._lock = threading.Lock()
        self._status = TaskStatus.IDLE

    def execute(self) -> Any:
        if self._work:
            self._work(self, *self._args)

    def get_id(self):
        return self._id

    def get_description(self):
        return self._description

    def is_condition_met(self):
        return self._condition(self, *self._args)

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


class TaskPhase(Enum):
    CLEAN = "CLEAN"
    COMPILE = "COMPILE"
    CONFIGURE = "CONFIGURE"
    FETCH = "FETCH"
    POST_COMPILE = "POST_COMPILE"
    POST_CONFIGURE = "POST_CONFIGURE"
    PRE_COMPILE = "PRE_COMPILE"
    PRE_VALIDATE = "PRE_VALIDATE"
    TEST = "TEST"
    VALIDATE = "VALIDATE"


TASK_PHASE_ORDER = (
    TaskPhase.CLEAN,
    TaskPhase.PRE_VALIDATE,
    TaskPhase.VALIDATE,
    TaskPhase.FETCH,
    TaskPhase.CONFIGURE,
    TaskPhase.POST_CONFIGURE,
    TaskPhase.PRE_COMPILE,
    TaskPhase.COMPILE,
    TaskPhase.POST_COMPILE,
    TaskPhase.TEST
)


class TaskStatus(Enum):
    IDLE = "IDLE"
    STARTED = "STARTED"
    FINISHED = "FINISHED"
    FAILED = "FAILED"
    CANCELED = "CANCELED"


class TaskRunner:
    _collections: Mapping[TaskPhase, List[TaskCollection]]
    _max_workers: Union[int, None]

    def __init__(self, max_workers: int = None):
        self._collections = dict((phase, []) for phase in TaskPhase)
        self._max_workers = max_workers

    def execute(self, on_status: Callable[[TaskStatus, str, bool, str, Exception], None] = lambda *_: None):
        shared = TaskWorkShared(
            stop=False,
            status_queue=queue.Queue(),
            lock=threading.Lock()
        )

        status = None
        force_stop = False
        for phase in TASK_PHASE_ORDER:
            if force_stop:
                break
            for collection in self._collections[phase]:
                if force_stop:
                    break
                tasks = collection.get_tasks()
                if len(tasks) == 0:
                    continue
                is_concurrent = collection.is_concurrent()
                worker_count = self._max_workers if is_concurrent else 1
                with ThreadPoolExecutor(max_workers=worker_count) as executor:
                    per_collection = TaskWorkPerCollection(
                        task_count=len(tasks),
                        tasks_done=0
                    )
                    for task in tasks:
                        executor.submit(self._worker, shared,
                                        per_collection, task)
                    while not force_stop:
                        with shared.lock:
                            done = per_collection.tasks_done >= per_collection.task_count
                        if done and shared.status_queue.empty():
                            break
                        task, status = shared.status_queue.get()
                        output = task.get_result()
                        e = task.get_exception()
                        on_status(status, task.get_description(),
                                  is_concurrent, output, e)
                        if status == TaskStatus.FAILED:
                            force_stop = True
                    if force_stop:
                        executor.shutdown(wait=False, cancel_futures=True)

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
            status = TaskStatus.FINISHED
        except Exception as e:
            status = TaskStatus.FAILED
            task.set_exception(e)
            with shared.lock:
                shared.stop = True
        task.set_status(status)
        per_collection.one_done()
        shared.status_queue.put((task, status))

    def get_task_count(self):
        sum = 0
        for phase in TaskPhase:
            for collections in self._collections[phase]:
                sum += collections.get_count()
        return sum

    def create_task_collection(self, phase: TaskPhase, concurrent: bool = False):
        collection = TaskCollection(concurrent=concurrent)
        self.add_task_collection(phase, collection)
        return collection

    def add_task_collection(self, phase: TaskPhase, *collections: TaskCollection):
        self._collections[phase] += collections
