/*
 * layered_test_fixtures.h
 *
 * Native C++ layered items for the binding tests: a sigma coordinate whose
 * free surface the test can move, two channel cross-sections with closed
 * forms to check against, a layered network that carries them, and a
 * time-varying layered mesh. Python reaches them exactly as it would reach a
 * loaded component's items -- as a plain IComponentDataItem -- so the tests
 * exercise the bindings' cross-casts, not a back door.
 */
#pragma once

#include "hydrocouple.h"
#include "hydrocouplespatial.h"
#include "hydrocouplespatiotemporal.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace HydroCouple {
namespace Python {
namespace Testing {

// -- Boilerplate shared by the fixtures ------------------------------------

/// Signals are accepted and ignored; nothing here emits.
template <typename Interface>
class QuietIdentity : public virtual Interface
{
public:
    explicit QuietIdentity(std::string id) : m_id(std::move(id)), m_caption(m_id) {}

    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void blockSignals(bool) override {}

    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

protected:
    void emit(std::string) override {}

private:
    std::string m_id;
    std::string m_caption;
    std::string m_description;
};

class FixtureDimension final : public QuietIdentity<IDimension>
{
public:
    FixtureDimension(std::string id, DimensionRole role)
        : QuietIdentity<IDimension>(std::move(id)), m_role(role) {}

    [[nodiscard]] LengthType lengthType() const override { return LengthType::Static; }
    [[nodiscard]] DimensionRole role() const override { return m_role; }

private:
    DimensionRole m_role;
};

/// The IComponentDataItem part of every fixture item. Values are not the
/// point of these fixtures, so the data plane refuses politely.
class FixtureItemBase : public QuietIdentity<IComponentDataItem>
{
public:
    FixtureItemBase(std::string id, std::vector<int64_t> shape,
                    std::vector<IDimension *> dimensions)
        : QuietIdentity<IComponentDataItem>(std::move(id)), m_shape(std::move(shape)),
          m_dimensions(std::move(dimensions)) {}

    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    using QuietIdentity<IComponentDataItem>::connect;
    using QuietIdentity<IComponentDataItem>::disconnect;

    [[nodiscard]] IModelComponent *modelComponent() const override { return nullptr; }
    [[nodiscard]] std::vector<IDimension *> dimensions() const override { return m_dimensions; }
    [[nodiscard]] std::vector<int64_t> shape() const override { return m_shape; }
    [[nodiscard]] DataKind dataKind() const override { return DataKind::Float64; }
    [[nodiscard]] IValueDefinition *valueDefinition() const override { return nullptr; }

    [[nodiscard]] bool getValuesInto(const BufferDescriptor &, std::span<const int64_t>,
                                     std::span<const int64_t>,
                                     std::string *message = nullptr) const override
    {
        if (message)
            *message = "fixture holds no values";
        return false;
    }

    [[nodiscard]] bool setValuesFrom(const BufferDescriptor &, std::span<const int64_t>,
                                     std::span<const int64_t>,
                                     std::string *message = nullptr) override
    {
        if (message)
            *message = "fixture holds no values";
        return false;
    }

protected:
    void emit(const std::shared_ptr<IComponentDataItemValueChanged> &) override {}
    using QuietIdentity<IComponentDataItem>::emit;

private:
    std::vector<int64_t>      m_shape;
    std::vector<IDimension *> m_dimensions;
};

// -- Vertical coordinate ----------------------------------------------------

/// Sigma layers over a flat bed. Interface k of every column sits at
/// stage + (k / layers) * (bed - stage). The profile is allocated once and
/// rewritten in place when the stage moves, so a zero-copy view of it sees
/// the move -- which is what the binding test checks.
class FixtureSigmaCoordinate final : public Spatial::IVerticalCoordinate
{
public:
    FixtureSigmaCoordinate(int64_t columns, int64_t layers, double bed, double stage)
        : m_columns(columns), m_layers(layers), m_bed(bed),
          m_profile(static_cast<size_t>(columns * (layers + 1)))
    {
        setStage(stage);
    }

    void setStage(double stage)
    {
        m_stage = stage;
        for (int64_t c = 0; c < m_columns; ++c)
            for (int64_t k = 0; k <= m_layers; ++k)
                m_profile[static_cast<size_t>(c * (m_layers + 1) + k)] =
                    interfaceElevation(c, k);
        ++m_epoch;
    }

    [[nodiscard]] Spatial::VerticalCoordinateKind kind() const override
    {
        return Spatial::VerticalCoordinateKind::Sigma;
    }
    [[nodiscard]] int64_t layerCount() const override { return m_layers; }
    [[nodiscard]] int64_t columnCount() const override { return m_columns; }
    [[nodiscard]] bool isTimeVarying() const override { return true; }
    [[nodiscard]] uint64_t geometryEpoch() const override { return m_epoch; }

    [[nodiscard]] double interfaceElevation(int64_t cellIndex,
                                            int64_t interfaceIndex) const override
    {
        if (cellIndex < 0 || cellIndex >= m_columns || interfaceIndex < 0 ||
            interfaceIndex > m_layers)
            throw std::out_of_range("interfaceElevation: index out of range");
        const double sigma =
            static_cast<double>(interfaceIndex) / static_cast<double>(m_layers);
        return m_stage + sigma * (m_bed - m_stage);
    }

    [[nodiscard]] std::span<const double> interfaceElevations() const override
    {
        return m_profile;
    }

private:
    int64_t             m_columns;
    int64_t             m_layers;
    double              m_bed;
    double              m_stage = 0.0;
    uint64_t            m_epoch = 0;
    std::vector<double> m_profile;
};

// -- Cross-sections ---------------------------------------------------------

/// Shared evaluate(): the per-stage accessors, tabulated.
class FixtureSectionBase : public Spatial::ICrossSection
{
public:
    void evaluate(std::span<const double> stages, std::span<double> topWidths,
                  std::span<double> storageAreas, std::span<double> flowAreas,
                  std::span<double> wettedPerimeters) const override
    {
        for (std::span<double> out : {topWidths, storageAreas, flowAreas, wettedPerimeters})
            if (!out.empty() && out.size() != stages.size())
                throw std::invalid_argument("evaluate: output length differs from stages");
        for (size_t i = 0; i < stages.size(); ++i)
        {
            if (!topWidths.empty())
                topWidths[i] = topWidth(stages[i]);
            if (!storageAreas.empty())
                storageAreas[i] = storageArea(stages[i]);
            if (!flowAreas.empty())
                flowAreas[i] = flowArea(stages[i]);
            if (!wettedPerimeters.empty())
                wettedPerimeters[i] = wettedPerimeter(stages[i]);
        }
    }
};

/// A surveyed trapezoid: bottom width 4, side slopes 1 horizontal to 1
/// vertical, banks 2 above the invert, vertical walls above the banks.
/// Surveyed as four points. A quarter of the wetted area is ineffective, so
/// flowArea() and storageArea() differ and a test can tell them apart.
class FixtureTrapezoidSection final : public FixtureSectionBase
{
public:
    explicit FixtureTrapezoidSection(double invert) : m_invert(invert) {}

    static constexpr double Bottom = 4.0;
    static constexpr double Bank   = 2.0;

    [[nodiscard]] Spatial::CrossSectionKind kind() const override
    {
        return Spatial::CrossSectionKind::StationElevation;
    }
    [[nodiscard]] double invertElevation() const override { return m_invert; }

    [[nodiscard]] double topWidth(double stage) const override
    {
        const double y = stage - m_invert;
        if (y < 0.0)
            return 0.0;
        return y <= Bank ? Bottom + 2.0 * y : Bottom + 2.0 * Bank;
    }

    [[nodiscard]] double storageArea(double stage) const override
    {
        const double y = stage - m_invert;
        if (y <= 0.0)
            return 0.0;
        if (y <= Bank)
            return (Bottom + y) * y;
        return (Bottom + Bank) * Bank + (Bottom + 2.0 * Bank) * (y - Bank);
    }

    [[nodiscard]] double flowArea(double stage) const override
    {
        return 0.75 * storageArea(stage);
    }

    [[nodiscard]] double wettedPerimeter(double stage) const override
    {
        const double y = stage - m_invert;
        if (y < 0.0)
            return 0.0;
        const double slope = std::sqrt(2.0);
        if (y <= Bank)
            return Bottom + 2.0 * slope * y;
        return Bottom + 2.0 * slope * Bank + 2.0 * (y - Bank);
    }

    [[nodiscard]] int64_t stationCount() const override { return 4; }

    void stations(double *stations, double *elevations) const override
    {
        const double s[4] = {0.0, Bank, Bank + Bottom, Bottom + 2.0 * Bank};
        const double z[4] = {m_invert + Bank, m_invert, m_invert, m_invert + Bank};
        for (int i = 0; i < 4; ++i)
        {
            stations[i]   = s[i];
            elevations[i] = z[i];
        }
    }

private:
    double m_invert;
};

/// A closed-form rectangle of width 5: no survey points at all.
class FixtureRectangleSection final : public FixtureSectionBase
{
public:
    explicit FixtureRectangleSection(double invert) : m_invert(invert) {}

    static constexpr double Width = 5.0;

    [[nodiscard]] Spatial::CrossSectionKind kind() const override
    {
        return Spatial::CrossSectionKind::Analytic;
    }
    [[nodiscard]] double invertElevation() const override { return m_invert; }
    [[nodiscard]] double topWidth(double stage) const override
    {
        return stage < m_invert ? 0.0 : Width;
    }
    [[nodiscard]] double storageArea(double stage) const override
    {
        return stage <= m_invert ? 0.0 : Width * (stage - m_invert);
    }
    [[nodiscard]] double flowArea(double stage) const override { return storageArea(stage); }
    [[nodiscard]] double wettedPerimeter(double stage) const override
    {
        return stage < m_invert ? 0.0 : Width + 2.0 * (stage - m_invert);
    }
    [[nodiscard]] int64_t stationCount() const override { return 0; }
    void stations(double *, double *) const override {}

private:
    double m_invert;
};

// -- Layered items ------------------------------------------------------------

/// Three nodes of a stratified channel, four layers each; a surveyed
/// section at node 0, none at node 1, a closed-form one at node 2.
class FixtureLayeredNetwork final : public FixtureItemBase,
                                    public virtual Spatial::ILayeredNetworkComponentDataItem
{
public:
    FixtureLayeredNetwork(Spatial::IVerticalCoordinate *vertical, IDimension *entities,
                          IDimension *layers, Spatial::ICrossSection *atNode0,
                          Spatial::ICrossSection *atNode2)
        : FixtureItemBase("layered_network",
                          {vertical->columnCount(), vertical->layerCount()},
                          {entities, layers}),
          m_vertical(vertical), m_entities(entities), m_layers(layers),
          m_sections{atNode0, nullptr, atNode2}
    {
    }

    [[nodiscard]] Spatial::INetwork *network() const override { return nullptr; }
    [[nodiscard]] Spatial::MeshLocation location() const override
    {
        return Spatial::MeshLocation::Node;
    }
    [[nodiscard]] Spatial::SpatialDataType networkDataType() const override
    {
        return Spatial::SpatialDataType::Scalar;
    }
    [[nodiscard]] Spatial::VectorBasis vectorBasis() const override
    {
        return Spatial::VectorBasis::Unknown;
    }
    [[nodiscard]] IDimension *entityDimension() const override { return m_entities; }
    [[nodiscard]] IDimension *layerDimension() const override { return m_layers; }
    [[nodiscard]] Spatial::IVerticalCoordinate *verticalCoordinate() const override
    {
        return m_vertical;
    }
    [[nodiscard]] Spatial::ICrossSection *crossSection(int64_t entityIndex) const override
    {
        if (entityIndex < 0 || entityIndex >= 3)
            throw std::out_of_range("crossSection: entity index out of range");
        return m_sections[static_cast<size_t>(entityIndex)];
    }

private:
    Spatial::IVerticalCoordinate *m_vertical;
    IDimension                   *m_entities;
    IDimension                   *m_layers;
    Spatial::ICrossSection       *m_sections[3];
};

/// A layered mesh stepping through time: {time, volume, layer}.
class FixtureTimeLayeredMesh final
    : public FixtureItemBase,
      public virtual SpatioTemporal::ITimeLayeredMeshComponentDataItem
{
public:
    FixtureTimeLayeredMesh(Spatial::IVerticalCoordinate *vertical, IDimension *time,
                           IDimension *entities, IDimension *layers)
        : FixtureItemBase("time_layered_mesh",
                          {2, vertical->columnCount(), vertical->layerCount()},
                          {time, entities, layers}),
          m_vertical(vertical), m_time(time), m_entities(entities), m_layers(layers)
    {
    }

    // ITimeSeriesComponentDataItem
    [[nodiscard]] const Temporal::IDateTime *time(int64_t) const override { return nullptr; }
    [[nodiscard]] int64_t timeCount() const override
    {
        return static_cast<int64_t>(m_times.size());
    }
    [[nodiscard]] std::span<const double> times() const override { return m_times; }
    [[nodiscard]] Temporal::ITimeSpan *timeSpan() const override { return nullptr; }
    [[nodiscard]] IDimension *timeDimension() const override { return m_time; }
    [[nodiscard]] Temporal::TimeKind timeKind() const override
    {
        return Temporal::TimeKind::Instantaneous;
    }
    [[nodiscard]] double intervalLength() const override { return 0.0; }
    [[nodiscard]] Temporal::TimeInterpolation timeInterpolation() const override
    {
        return Temporal::TimeInterpolation::None;
    }
    [[nodiscard]] Temporal::TimeExtrapolation timeExtrapolation() const override
    {
        return Temporal::TimeExtrapolation::Refuse;
    }

    // IPolyhedralSurfaceComponentDataItem
    [[nodiscard]] Spatial::MeshLocation location() const override
    {
        return Spatial::MeshLocation::Volume;
    }
    [[nodiscard]] Spatial::SpatialDataType meshDataType() const override
    {
        return Spatial::SpatialDataType::Scalar;
    }
    [[nodiscard]] Spatial::VectorBasis vectorBasis() const override
    {
        return Spatial::VectorBasis::Unknown;
    }
    [[nodiscard]] Spatial::IPolyhedralSurface *polyhedralSurface() const override
    {
        return nullptr;
    }
    [[nodiscard]] IDimension *entityDimension() const override { return m_entities; }

    // ILayering
    [[nodiscard]] IDimension *layerDimension() const override { return m_layers; }
    [[nodiscard]] Spatial::IVerticalCoordinate *verticalCoordinate() const override
    {
        return m_vertical;
    }

private:
    Spatial::IVerticalCoordinate *m_vertical;
    IDimension                   *m_time;
    IDimension                   *m_entities;
    IDimension                   *m_layers;
    std::vector<double>           m_times{2461000.5, 2461000.75};
};

/// Owns one of everything. Items are handed out as plain IComponentDataItem
/// pointers, the way a component's outputs reach the bindings.
class LayeredFixtures
{
public:
    static constexpr int64_t Columns = 3;
    static constexpr int64_t Layers  = 4;
    static constexpr double  Bed     = -10.0;
    static constexpr double  Stage   = 2.0;
    static constexpr double  Invert  = 1.0;

    LayeredFixtures()
        : m_vertical(Columns, Layers, Bed, Stage),
          m_entities("nodes", IDimension::DimensionRole::Entity),
          m_layers("layers", IDimension::DimensionRole::Layer),
          m_time("time", IDimension::DimensionRole::Time),
          m_trapezoid(Invert),
          m_rectangle(Invert),
          m_network(&m_vertical, &m_entities, &m_layers, &m_trapezoid, &m_rectangle),
          m_timeMesh(&m_vertical, &m_time, &m_entities, &m_layers),
          m_plain("plain", {Columns}, {&m_entities})
    {
    }

    IComponentDataItem *network() { return &m_network; }
    IComponentDataItem *timeMesh() { return &m_timeMesh; }
    IComponentDataItem *plain() { return &m_plain; }
    void setStage(double stage) { m_vertical.setStage(stage); }

private:
    FixtureSigmaCoordinate  m_vertical;
    FixtureDimension        m_entities;
    FixtureDimension        m_layers;
    FixtureDimension        m_time;
    FixtureTrapezoidSection m_trapezoid;
    FixtureRectangleSection m_rectangle;
    FixtureLayeredNetwork   m_network;
    FixtureTimeLayeredMesh  m_timeMesh;
    FixtureItemBase         m_plain;
};

} // namespace Testing
} // namespace Python
} // namespace HydroCouple
