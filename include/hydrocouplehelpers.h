/*!
 * \file hydrocouplehelpers.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0-alpha.1
 * \brief Non-normative convenience helpers for the HydroCouple interface standard.
 * \details This header is the single deliberate exception to the standard's
 * no-implementation rule: the interface headers (hydrocouple.h and companions)
 * contain only pure declarations, enums, and plain aggregates, while the small
 * executable conveniences that nearly every implementer and consumer would
 * otherwise rewrite — DataKind metadata, BufferDescriptor arithmetic, the
 * lifecycle transition table, and typed wrappers over the hyperslab API — live
 * here. Nothing in this header is required to implement or consume the standard;
 * it may be ignored entirely.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute them and/or modify them under the terms of the
 * MIT License. They are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2026, Caleb Buahin, All rights reserved.
 * \date 2014-2026
 */

#ifndef HYDROCOUPLEHELPERS_H
#define HYDROCOUPLEHELPERS_H

#include "hydrocouple.h"

namespace HydroCouple
{
  //! Non-normative convenience helpers for the HydroCouple interface standard.
  namespace Helpers
  {
    /*!
     * \brief Size in bytes of one element of the given DataKind.
     * \param[in] kind is the element kind to query.
     * \returns The element size in bytes; 0 for String, Opaque, and Unknown.
     */
    constexpr int64_t dataKindSize(DataKind kind)
    {
      switch (kind)
      {
        case DataKind::Int8:
        case DataKind::UInt8:
        case DataKind::Boolean: return 1;
        case DataKind::Int16:
        case DataKind::UInt16:  return 2;
        case DataKind::Int32:
        case DataKind::UInt32:
        case DataKind::Float32: return 4;
        case DataKind::Int64:
        case DataKind::UInt64:
        case DataKind::Float64: return 8;
        default:                return 0;
      }
    }

    /*!
     * \brief DataKindOf maps a C++ type to its DataKind at compile time.
     * \tparam T is the C++ element type.
     */
    template <typename T>
    struct DataKindOf { static constexpr DataKind kind = DataKind::Opaque; };
    template <> struct DataKindOf<int8_t>      { static constexpr DataKind kind = DataKind::Int8; };
    template <> struct DataKindOf<uint8_t>     { static constexpr DataKind kind = DataKind::UInt8; };
    template <> struct DataKindOf<int16_t>     { static constexpr DataKind kind = DataKind::Int16; };
    template <> struct DataKindOf<uint16_t>    { static constexpr DataKind kind = DataKind::UInt16; };
    template <> struct DataKindOf<int32_t>     { static constexpr DataKind kind = DataKind::Int32; };
    template <> struct DataKindOf<uint32_t>    { static constexpr DataKind kind = DataKind::UInt32; };
    template <> struct DataKindOf<int64_t>     { static constexpr DataKind kind = DataKind::Int64; };
    template <> struct DataKindOf<uint64_t>    { static constexpr DataKind kind = DataKind::UInt64; };
    template <> struct DataKindOf<float>       { static constexpr DataKind kind = DataKind::Float32; };
    template <> struct DataKindOf<double>      { static constexpr DataKind kind = DataKind::Float64; };
    template <> struct DataKindOf<bool>        { static constexpr DataKind kind = DataKind::Boolean; };
    template <> struct DataKindOf<std::string> { static constexpr DataKind kind = DataKind::String; };

    /*!
     * \brief Number of elements a BufferDescriptor describes (product of shape; 1 for rank 0).
     * \param[in] d is the descriptor to query.
     */
    [[nodiscard]] constexpr int64_t elementCount(const BufferDescriptor &d)
    {
      int64_t n = 1;
      for (int32_t k = 0; k < d.rank; ++k)
        n *= d.shape[k];
      return n;
    }

    /*!
     * \brief Checks whether a BufferDescriptor's layout is C-contiguous (row-major, packed).
     * \param[in] d is the descriptor to query.
     */
    [[nodiscard]] constexpr bool isContiguous(const BufferDescriptor &d)
    {
      if (d.stridesBytes == nullptr)
        return true;
      int64_t expected = dataKindSize(d.kind);
      for (int32_t k = d.rank - 1; k >= 0; --k)
      {
        if (d.shape[k] != 1 && d.stridesBytes[k] != expected)
          return false;
        expected *= d.shape[k];
      }
      return true;
    }

    /*!
     * \brief Byte offset from a descriptor's base address of the element at the given index.
     * \param[in] d is the descriptor to query.
     * \param[in] index is the multi-dimensional element index; its length must equal d.rank.
     */
    [[nodiscard]] constexpr int64_t byteOffset(const BufferDescriptor &d, std::span<const int64_t> index)
    {
      int64_t offset = 0;
      if (d.stridesBytes == nullptr)
      {
        int64_t stride = dataKindSize(d.kind);
        for (int32_t k = d.rank - 1; k >= 0; --k)
        {
          offset += index[k] * stride;
          stride *= d.shape[k];
        }
      }
      else
      {
        for (int32_t k = 0; k < d.rank; ++k)
          offset += index[k] * d.stridesBytes[k];
      }
      return offset;
    }

    /*!
     * \brief Total byte extent of a C-contiguous buffer of the descriptor's shape and kind.
     * \param[in] d is the descriptor to query.
     */
    [[nodiscard]] constexpr int64_t contiguousByteSize(const BufferDescriptor &d)
    {
      return elementCount(d) * dataKindSize(d.kind);
    }

    /*!
     * \brief Builds a C-contiguous host BufferDescriptor.
     * \param[in] data is the base address of the buffer.
     * \param[in] kind is the element type.
     * \param[in] shape is the extent per dimension; the pointed-to array must outlive the descriptor.
     */
    [[nodiscard]] constexpr BufferDescriptor makeContiguous(void *data, DataKind kind, std::span<const int64_t> shape)
    {
      BufferDescriptor d;
      d.data = data;
      d.kind = kind;
      d.rank = static_cast<int32_t>(shape.size());
      d.shape = shape.data();
      return d;
    }

    /*!
     * \brief Encodes the normative component lifecycle state machine.
     * \details Implementations must not perform status transitions for which this
     * function returns false.
     * \param[in] from is the current status.
     * \param[in] to is the proposed next status.
     * \returns True if the transition is legal.
     */
    [[nodiscard]] constexpr bool isValidComponentStatusTransition(IModelComponent::ComponentStatus from,
                                                                  IModelComponent::ComponentStatus to)
    {
      using CS = IModelComponent::ComponentStatus;
      switch (from)
      {
        case CS::Created:        return to == CS::Initializing;
        case CS::Initializing:   return to == CS::Initialized || to == CS::Failed;
        case CS::Initialized:    return to == CS::Validating || to == CS::Initializing;
        case CS::Validating:     return to == CS::Valid || to == CS::Invalid;
        case CS::Valid:          return to == CS::Preparing || to == CS::Validating;
        case CS::Invalid:        return to == CS::Validating;
        case CS::Preparing:      return to == CS::Updated || to == CS::Failed;
        case CS::Updating:       return to == CS::Updated || to == CS::Done ||
                                        to == CS::WaitingForData || to == CS::Failed;
        case CS::WaitingForData: return to == CS::Updating || to == CS::Failed;
        case CS::Updated:        return to == CS::Updating || to == CS::Checkpointing ||
                                        to == CS::Finishing;
        case CS::Checkpointing:  return to == CS::Updated || to == CS::Failed;
        case CS::Done:           return to == CS::Finishing;
        case CS::Finishing:      return to == CS::Finished || to == CS::Created;
        case CS::Finished:       return false;
        case CS::Failed:         return to == CS::Initializing || to == CS::Finishing;
      }
      return false;
    }

    /*!
     * \brief Typed convenience: reads the single element at the given index of a data item.
     * \tparam T is the C++ element type; DataKindOf<T>::kind must equal item.dataKind().
     * \param[in] item is the data item to read from.
     * \param[out] value receives the element.
     * \param[in] index is the full multi-dimensional element index.
     * \returns True on success.
     */
    template <typename T>
    [[nodiscard]] bool getValue(const IComponentDataItem &item, T &value, std::span<const int64_t> index)
    {
      std::vector<int64_t> ones(index.size(), 1);
      const int64_t one = 1;
      return item.getValuesInto(makeContiguous(&value, DataKindOf<T>::kind, std::span<const int64_t>(&one, 1)),
                                index, ones);
    }

    /*!
     * \brief Typed convenience: writes the single element at the given index of a data item.
     * \tparam T is the C++ element type; DataKindOf<T>::kind must equal item.dataKind().
     * \param[in] item is the data item to write to.
     * \param[in] value is the element to write.
     * \param[in] index is the full multi-dimensional element index.
     * \returns True on success.
     */
    template <typename T>
    [[nodiscard]] bool setValue(IComponentDataItem &item, const T &value, std::span<const int64_t> index)
    {
      std::vector<int64_t> ones(index.size(), 1);
      const int64_t one = 1;
      return item.setValuesFrom(makeContiguous(const_cast<T *>(&value), DataKindOf<T>::kind, std::span<const int64_t>(&one, 1)),
                                index, ones);
    }

    /*!
     * \brief Typed convenience: reads a hyperslab into a flat contiguous span in C order.
     * \tparam T is the C++ element type; DataKindOf<T>::kind must equal item.dataKind().
     * \param[in] item is the data item to read from.
     * \param[out] destination receives the selection; its size must equal the product of count.
     * \param[in] start is the first index of the selection in each dimension.
     * \param[in] count is the selection extent in each dimension.
     * \returns True on success.
     */
    template <typename T>
    [[nodiscard]] bool getValues(const IComponentDataItem &item, std::span<T> destination,
                                 std::span<const int64_t> start, std::span<const int64_t> count)
    {
      return item.getValuesInto(makeContiguous(destination.data(), DataKindOf<T>::kind, count), start, count);
    }

    /*!
     * \brief Typed convenience: writes a hyperslab from a flat contiguous span in C order.
     * \tparam T is the C++ element type; DataKindOf<T>::kind must equal item.dataKind().
     * \param[in] item is the data item to write to.
     * \param[in] source provides the selection; its size must equal the product of count.
     * \param[in] start is the first index of the selection in each dimension.
     * \param[in] count is the selection extent in each dimension.
     * \returns True on success.
     */
    template <typename T>
    [[nodiscard]] bool setValues(IComponentDataItem &item, std::span<const T> source,
                                 std::span<const int64_t> start, std::span<const int64_t> count)
    {
      return item.setValuesFrom(makeContiguous(const_cast<T *>(source.data()), DataKindOf<T>::kind, count), start, count);
    }
  }
}

#endif // HYDROCOUPLEHELPERS_H
