/*!
 * \file hydrocoupledistributed.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0-alpha.1
 * \brief Distributed-execution interface definitions for the HydroCouple component-based modeling framework.
 * \details This header file contains the transport-neutral interfaces for distributed
 * and cloud execution: message transports, distributed model components, proxies for
 * remote components, and partitioned data items with local virtual (ghost/halo)
 * entity representation. MPI is one possible transport backend; nothing in this
 * standard names an MPI type, so the same interfaces serve TCP/gRPC/UCX transports
 * in cloud deployments.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute them and/or modify them under the terms of the
 * MIT License. They are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2026, Caleb Buahin, All rights reserved.
 * \date 2014-2026
 */

#ifndef HYDROCOUPLEDISTRIBUTED_H
#define HYDROCOUPLEDISTRIBUTED_H

#include "hydrocouple.h"

namespace HydroCouple
{
  //! HydroCouple's interfaces for distributed and cloud execution.
  namespace Distributed
  {
    /*!
     * \brief IExchangeRequest is a handle to an asynchronous communication operation.
     * \details Returned by the asynchronous ITransport operations and by
     * IPartitionedComponentDataItem::synchronizeAsync(). Completion machinery
     * (progress threads, transport polling) is owned by the implementing SDK;
     * consumers only test or wait.
     */
    class IExchangeRequest
    {
    public:
      /*!
       * \brief IExchangeRequest::~IExchangeRequest is a virtual destructor.
       * Destroying an incomplete request cancels it if the implementation supports
       * cancellation, otherwise blocks until completion.
       */
      virtual ~IExchangeRequest() = default;

      /*!
       * \brief Non-blocking completion test.
       * \returns True if the operation has completed (successfully or not).
       */
      [[nodiscard]] virtual bool test() = 0;

      /*!
       * \brief Blocks until the operation completes.
       */
      virtual void wait() = 0;

      /*!
       * \brief Checks whether the operation completed with an error.
       * \param[out] message describes the failure when the return value is true.
       * \returns True if the operation failed.
       */
      [[nodiscard]] virtual bool failed(std::string &message) const = 0;
    };

    /*!
     * \brief ITransport is a transport-neutral, tagged message channel between processes.
     *
     * \details Implementations are provided by SDKs: an MPI transport (built on
     * communicators/inter-communicators), an in-process transport (for testing
     * distributed compositions without a launcher), or socket-based transports for
     * cloud execution. Payloads are always typed BufferDescriptor views, so the bytes
     * on the wire have the same layout discipline as the bytes in memory.
     *
     * \details Threading contract: send/receive and their asynchronous variants are
     * safe to call from multiple threads; message ordering is guaranteed only between
     * a fixed (sender, receiver, tag) triple.
     */
    class ITransport : public virtual IIdentity
    {
    public:
      /*!
       * \brief Endpoint identifies one participant reachable through this transport.
       */
      struct Endpoint
      {
        std::string address; //!< Transport-specific address (empty for rank-addressed transports).
        int32_t     rank = -1; //!< Participant rank for rank-addressed transports; -1 otherwise.
      };

      /*!
       * \brief ITransport::~ITransport is a virtual destructor.
       */
      virtual ~ITransport() = default;

      /*!
       * \brief The endpoint of the calling process on this transport.
       * \returns The local Endpoint.
       */
      [[nodiscard]] virtual Endpoint localEndpoint() const = 0;

      /*!
       * \brief The number of participants reachable through this transport.
       * \returns The participant count.
       */
      [[nodiscard]] virtual int32_t participantCount() const = 0;

      /*!
       * \brief Sends a typed payload to a peer, blocking until the payload buffer is reusable.
       * \param[in] to is the destination endpoint.
       * \param[in] tag disambiguates concurrent message streams between the same pair of endpoints.
       * \param[in] payload describes the bytes to send.
       * \param[out] message optionally receives a failure description.
       * \returns True on success.
       */
      [[nodiscard]] virtual bool send(const Endpoint &to, int32_t tag, const BufferDescriptor &payload, std::string *message = nullptr) = 0;

      /*!
       * \brief Receives a typed payload from a peer, blocking until it arrives.
       * \param[in] from is the source endpoint.
       * \param[in] tag is the message stream tag to match.
       * \param[in] into describes the pre-allocated buffer receiving the payload.
       * \param[out] message optionally receives a failure description.
       * \returns True on success.
       */
      [[nodiscard]] virtual bool receive(const Endpoint &from, int32_t tag, const BufferDescriptor &into, std::string *message = nullptr) = 0;

      /*!
       * \brief Nonblocking send; the payload buffer must remain valid until the request completes.
       * \param[in] to is the destination endpoint.
       * \param[in] tag disambiguates concurrent message streams between the same pair of endpoints.
       * \param[in] payload describes the bytes to send.
       * \returns A request handle; never nullptr.
       */
      [[nodiscard]] virtual std::unique_ptr<IExchangeRequest> sendAsync(const Endpoint &to, int32_t tag, const BufferDescriptor &payload) = 0;

      /*!
       * \brief Nonblocking receive; the destination buffer must remain valid until the request completes.
       * \param[in] from is the source endpoint.
       * \param[in] tag is the message stream tag to match.
       * \param[in] into describes the pre-allocated buffer receiving the payload.
       * \returns A request handle; never nullptr.
       */
      [[nodiscard]] virtual std::unique_ptr<IExchangeRequest> receiveAsync(const Endpoint &from, int32_t tag, const BufferDescriptor &into) = 0;
    };

    /*!
     * \brief IDistributedModelComponent is an IModelComponent that participates in
     * distributed execution through a transport.
     * \details Components implementing this interface advertise
     * Capability::DistributedExecution. This interface replaces the MPI-specific
     * methods (mpiProcessRank, mpiAllocateProcesses, ...) that earlier revisions
     * carried on IModelComponent itself.
     */
    class IDistributedModelComponent : public virtual IModelComponent
    {
    public:
      /*!
       * \brief ~IDistributedModelComponent destructor.
       */
      virtual ~IDistributedModelComponent() = default;

      /*!
       * \brief The transport this component communicates through.
       * \returns The ITransport used by this component.
       */
      [[nodiscard]] virtual ITransport *transport() const = 0;

      /*!
       * \brief The number of partitions this component's domain is decomposed into.
       * \returns The partition count; 1 for an undecomposed component.
       */
      [[nodiscard]] virtual int32_t partitionCount() const = 0;

      /*!
       * \brief The partition this instance computes.
       * \returns The zero-based partition rank of this instance.
       */
      [[nodiscard]] virtual int32_t partitionRank() const = 0;
    };

    /*!
     * \brief The IProxyModelComponent class is a local stand-in for a remote
     * IModelComponent reachable through a transport.
     *
     * \details Every IModelComponent method on the proxy forwards to the remote peer:
     * lifecycle calls are relayed as commands, status() is kept current asynchronously
     * from the peer's status notifications, and the proxy's exchange items move values
     * across the transport. An orchestrator therefore cannot — and need not —
     * distinguish a proxy from a local component.
     *
     * \details Failure semantics: if the peer dies or a request exceeds the configured
     * timeout, the proxy transitions to HydroCouple::ComponentStatus::Failed, queues a
     * Severity::Fatal ErrorEntry describing the loss, and fires its status-changed signal.
     */
    class IProxyModelComponent : public virtual IDistributedModelComponent
    {

    public:
      /*!
       * \brief ~IProxyModelComponent destructor.
       */
      virtual ~IProxyModelComponent() = default;

      /*!
       * \brief The endpoint of the remote component this proxy represents.
       * \returns The remote Endpoint.
       */
      [[nodiscard]] virtual ITransport::Endpoint remoteEndpoint() const = 0;

      /*!
       * \brief The unique identifier of the remote component this proxy represents.
       * \returns The remote component's id.
       */
      [[nodiscard]] virtual std::string remoteId() const = 0;

      /*!
       * \brief Establishes the connection to the remote component.
       * \param[out] message describes the failure when the return value is false.
       * \returns True on success.
       */
      [[nodiscard]] virtual bool connect(std::string &message) = 0;

      /*!
       * \brief Closes the connection to the remote component.
       */
      virtual void disconnect() = 0;

      /*!
       * \brief Checks whether the proxy currently holds a live connection.
       * \returns True if connected.
       */
      [[nodiscard]] virtual bool isConnected() const = 0;

      /*!
       * \brief Liveness probe of the remote component.
       * \param[in] timeoutSeconds is the maximum time to wait for a reply.
       * \returns True if the peer replied within the timeout.
       */
      [[nodiscard]] virtual bool ping(double timeoutSeconds) = 0;

      /*!
       * \brief The timeout applied to forwarded requests.
       * \returns The request timeout in seconds; a non-positive value means wait indefinitely.
       */
      [[nodiscard]] virtual double requestTimeout() const = 0;

      /*!
       * \brief Sets the timeout applied to forwarded requests.
       * \param[in] seconds is the request timeout; a non-positive value means wait indefinitely.
       */
      virtual void setRequestTimeout(double seconds) = 0;
    };

    /*!
     * \brief IPartitionedComponentDataItem is an IComponentDataItem whose entity
     * dimension is decomposed across partitions, with local virtual (ghost/halo)
     * representation of remotely owned entities.
     *
     * \details The item's entity dimension indexes locally *resident* entities: first
     * the locally owned entities, then the virtual entities mirrored from other
     * partitions. Virtual entities carry no degrees of freedom of their own — they are
     * read-only mirrors whose values are overwritten by synchronization, never solved
     * locally, so decomposed quantities are not double-counted. Exchanged payloads are
     * not limited to a single state scalar: coupling that requires implicit stability
     * should exchange flux and sensitivity (e.g., dQ/dh) pairs as additional value
     * dimensions.
     *
     * \details Synchronization is epoch-based: a completed halo exchange increments
     * synchronizationEpoch(), and consumers comparing epochs can detect stale mirrors
     * without inspecting values. Components exposing partitioned items advertise
     * Capability::PartitionedData.
     */
    class IPartitionedComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief ~IPartitionedComponentDataItem destructor.
       */
      virtual ~IPartitionedComponentDataItem() = default;

      /*!
       * \brief The global entity count across all partitions.
       * \returns The global number of entities in the decomposed dimension.
       */
      [[nodiscard]] virtual int64_t globalCount() const = 0;

      /*!
       * \brief Global identities of the locally owned entities.
       * \details The span is ordered consistently with the local entity dimension and
       * remains valid until the decomposition changes.
       * \returns Global indexes of locally owned entities.
       */
      [[nodiscard]] virtual std::span<const int64_t> ownedGlobalIndexes() const = 0;

      /*!
       * \brief Global identities of the local virtual (ghost/halo) entities.
       * \details Virtual entities follow the owned entities in the local entity
       * dimension. The span remains valid until the decomposition changes.
       * \returns Global indexes of locally mirrored, remotely owned entities.
       */
      [[nodiscard]] virtual std::span<const int64_t> virtualGlobalIndexes() const = 0;

      /*!
       * \brief Owner partition of each virtual entity.
       * \returns A span parallel to virtualGlobalIndexes() giving the owning partition rank of each virtual entity.
       */
      [[nodiscard]] virtual std::span<const int32_t> virtualOwners() const = 0;

      /*!
       * \brief The synchronization epoch of the virtual entities.
       * \returns A monotonically increasing counter incremented each time a halo synchronization completes.
       */
      [[nodiscard]] virtual uint64_t synchronizationEpoch() const = 0;

      /*!
       * \brief Begins an asynchronous halo synchronization of the virtual entities.
       * \details Refreshes local mirrors from their owning partitions. The exchange
       * pattern is derivable from ownedGlobalIndexes()/virtualOwners() and should be
       * built once at prepare() time. Computation on owned entities may overlap the
       * returned request; virtual-entity values are defined only after completion.
       * \returns A request handle; never nullptr.
       */
      [[nodiscard]] virtual std::unique_ptr<IExchangeRequest> synchronizeAsync() = 0;
    };
  }
}

#endif // HYDROCOUPLEDISTRIBUTED_H
