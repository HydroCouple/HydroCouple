"""
HydroCouple distributed-execution abstract base classes.

Python ABC mirrors of the C++ HydroCouple v2.0.0 interfaces defined in
``hydrocoupledistributed.h``: transport-neutral message channels,
distributed model components, proxies for remote components, and
partitioned data items with local virtual (ghost/halo) entity
representation. Transport payloads are NumPy arrays — the Python-side
``BufferDescriptor``.

Transport *implementations* (MPI, in-process, sockets) are SDK territory;
this module only defines the contracts.
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from typing import TYPE_CHECKING

from hydrocouple.core import IComponentDataItem, IIdentity, IModelComponent

if TYPE_CHECKING:
    import numpy as np


class IExchangeRequest(ABC):
    """Handle to an asynchronous communication operation.

    Mirrors C++ ``Distributed::IExchangeRequest``. Returned by the
    asynchronous :class:`ITransport` operations and by
    :meth:`IPartitionedComponentDataItem.synchronize_async`.
    """

    @abstractmethod
    def test(self) -> bool:
        """Non-blocking completion test."""
        raise NotImplementedError

    @abstractmethod
    def wait(self) -> None:
        """Block until the operation completes."""
        raise NotImplementedError

    @abstractmethod
    def failed(self) -> tuple[bool, str]:
        """Whether the operation failed; returns ``(failed, message)``."""
        raise NotImplementedError


@dataclass
class Endpoint:
    """One participant reachable through a transport.

    Mirrors C++ ``Distributed::ITransport::Endpoint``.
    """

    address: str = ""
    rank: int = -1


class ITransport(IIdentity):
    """A transport-neutral, tagged message channel between processes.

    Mirrors C++ ``Distributed::ITransport``. Payloads are NumPy arrays;
    message ordering is guaranteed only between a fixed
    ``(sender, receiver, tag)`` triple.
    """

    @property
    @abstractmethod
    def local_endpoint(self) -> Endpoint:
        """The endpoint of the calling process."""
        raise NotImplementedError

    @property
    @abstractmethod
    def participant_count(self) -> int:
        """The number of participants reachable through this transport."""
        raise NotImplementedError

    @abstractmethod
    def send(self, to: Endpoint, tag: int, payload: "np.ndarray") -> tuple[bool, str]:
        """Send a typed payload, blocking until the buffer is reusable.

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError

    @abstractmethod
    def receive(self, source: Endpoint, tag: int, into: "np.ndarray") -> tuple[bool, str]:
        """Receive a typed payload into a pre-allocated array, blocking.

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError

    @abstractmethod
    def send_async(self, to: Endpoint, tag: int, payload: "np.ndarray") -> IExchangeRequest:
        """Nonblocking send; the payload must stay valid until completion."""
        raise NotImplementedError

    @abstractmethod
    def receive_async(self, source: Endpoint, tag: int, into: "np.ndarray") -> IExchangeRequest:
        """Nonblocking receive; ``into`` must stay valid until completion."""
        raise NotImplementedError


class IDistributedModelComponent(IModelComponent):
    """An :class:`~hydrocouple.core.IModelComponent` participating in
    distributed execution through a transport.

    Mirrors C++ ``Distributed::IDistributedModelComponent``; advertises
    :attr:`~hydrocouple.core.Capability.DistributedExecution`.
    """

    @property
    @abstractmethod
    def transport(self) -> ITransport:
        """The transport this component communicates through."""
        raise NotImplementedError

    @property
    @abstractmethod
    def partition_count(self) -> int:
        """Number of partitions the domain is decomposed into (1 if none)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def partition_rank(self) -> int:
        """The zero-based partition this instance computes."""
        raise NotImplementedError


class IProxyModelComponent(IDistributedModelComponent):
    """A local stand-in for a remote model component.

    Mirrors C++ ``Distributed::IProxyModelComponent``. Every
    :class:`~hydrocouple.core.IModelComponent` method forwards to the
    remote peer; an orchestrator cannot — and need not — distinguish a
    proxy from a local component. If the peer dies or a request times out,
    the proxy transitions to ``Failed``, queues a ``Fatal``
    :class:`~hydrocouple.core.ErrorEntry`, and fires its status signal.
    """

    @property
    @abstractmethod
    def remote_endpoint(self) -> Endpoint:
        """The endpoint of the remote component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def remote_id(self) -> str:
        """The unique identifier of the remote component."""
        raise NotImplementedError

    @abstractmethod
    def connect_remote(self) -> tuple[bool, str]:
        """Establish the connection to the remote component.

        :returns: ``(ok, message)``.

        .. note:: named ``connect_remote`` to avoid clashing with the
           inherited signal method ``connect(slot)``; mirrors C++
           ``connect()``.
        """
        raise NotImplementedError

    @abstractmethod
    def disconnect_remote(self) -> None:
        """Close the connection to the remote component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_connected(self) -> bool:
        """Whether the proxy currently holds a live connection."""
        raise NotImplementedError

    @abstractmethod
    def ping(self, timeout_seconds: float) -> bool:
        """Liveness probe of the remote component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def request_timeout(self) -> float:
        """Timeout applied to forwarded requests (non-positive = wait
        indefinitely)."""
        raise NotImplementedError

    @request_timeout.setter
    @abstractmethod
    def request_timeout(self, seconds: float) -> None:
        raise NotImplementedError


class IPartitionedComponentDataItem(IComponentDataItem):
    """A data item whose entity dimension is decomposed across partitions,
    with local virtual (ghost/halo) representation of remote entities.

    Mirrors C++ ``Distributed::IPartitionedComponentDataItem``. The entity
    dimension indexes locally resident entities: first the locally owned
    entities, then the virtual entities mirrored from other partitions.
    Virtual entities carry no degrees of freedom — they are read-only
    mirrors overwritten by synchronization, never solved locally.
    Components exposing partitioned items advertise
    :attr:`~hydrocouple.core.Capability.PartitionedData`.
    """

    @property
    @abstractmethod
    def global_count(self) -> int:
        """Global entity count across all partitions."""
        raise NotImplementedError

    @property
    @abstractmethod
    def owned_global_indexes(self) -> "np.ndarray":
        """Global identities of locally owned entities (int64 array)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def virtual_global_indexes(self) -> "np.ndarray":
        """Global identities of local virtual (ghost/halo) entities
        (int64 array); they follow the owned entities in the local entity
        dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def virtual_owners(self) -> "np.ndarray":
        """Owner partition rank of each virtual entity (int32 array,
        parallel to :attr:`virtual_global_indexes`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def synchronization_epoch(self) -> int:
        """Monotonically increasing counter, incremented each time a halo
        synchronization completes."""
        raise NotImplementedError

    @abstractmethod
    def synchronize_async(self) -> IExchangeRequest:
        """Begin an asynchronous halo synchronization of the virtual
        entities. Computation on owned entities may overlap the returned
        request; virtual-entity values are defined only after completion."""
        raise NotImplementedError
