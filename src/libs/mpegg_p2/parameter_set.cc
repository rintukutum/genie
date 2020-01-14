#include "parameter_set.h"
#include "util/exceptions.h"

#include <sstream>

#include "mpegg_p2/parameter_set/descriptor_configuration.h"
#include "mpegg_p2/parameter_set/descriptor_configuration_container.h"
#include "mpegg_p2/parameter_set/qv_coding_config.h"
#include "util/bitreader.h"
#include "util/bitwriter.h"
#include "util/make_unique.h"

// -----------------------------------------------------------------------------------------------------------------
namespace genie {
namespace mpegg_p2 {
ParameterSet::ParameterSet(util::BitReader &bitReader) : DataUnit(DataUnitType::PARAMETER_SET) {
    bitReader.read(10);  // ISO 23092-2 Section 3.1 table 3
    bitReader.read(22);
    parameter_set_ID = bitReader.read(8);
    parent_parameter_set_ID = bitReader.read(8);
    dataset_type = DatasetType(bitReader.read(4));
    alphabet_ID = genie::AlphabetID(bitReader.read(8));
    read_length = bitReader.read(24);
    number_of_template_segments_minus1 = bitReader.read(2);
    bitReader.read(6);
    max_au_data_unit_size = bitReader.read(29);
    pos_40_bits_flag = bitReader.read(1);
    qv_depth = bitReader.read(3);
    as_depth = bitReader.read(3);
    uint8_t num_classes = bitReader.read(4);
    for (size_t i = 0; i < num_classes; ++i) {
        class_IDs.push_back(genie::mpegg_rec::ClassType(bitReader.read(4)));
    }
    for (size_t i = 0; i < genie::getDescriptors().size(); ++i) {
        descriptors.emplace_back(
            util::make_unique<DescriptorConfigurationContainer>(num_classes, genie::GenDesc(i), bitReader));
    }
    uint8_t num_groups = bitReader.read(16);
    for (size_t i = 0; i < num_groups; ++i) {
        rgroup_IDs.emplace_back();
        char c = 0;
        do {
            c = bitReader.read(8);
            rgroup_IDs.back().push_back(c);
        } while (c);
    }
    multiple_alignments_flag = bitReader.read(1);
    spliced_reads_flag = bitReader.read(1);
    multiple_signature_base = bitReader.read(31);
    if (multiple_signature_base > 0) {
        u_signature_size = util::make_unique<uint8_t>(bitReader.read(6));
    }
    for (size_t i = 0; i < num_classes; ++i) {
        qv_coding_configs.emplace_back(QvCodingConfig::factory(bitReader));
    }
    crps_flag = bitReader.read(1);
    if (crps_flag) {
        parameter_set_crps = util::make_unique<ParameterSetCrps>(bitReader);
    }
    bitReader.flush();
}

//------------------------------------------------------------------------------------------------------------------

ParameterSet::ParameterSet(uint8_t _parameter_set_ID, uint8_t _parent_parameter_set_ID, DatasetType _dataset_type,
                           genie::AlphabetID _alphabet_id, uint32_t _read_length, bool _paired_end,
                           bool _pos_40_bits_flag, uint8_t _qv_depth, uint8_t _as_depth, bool _multiple_alignments_flag,
                           bool _spliced_reads_flag)
    : DataUnit(DataUnitType::PARAMETER_SET),
      parameter_set_ID(_parameter_set_ID),
      parent_parameter_set_ID(_parent_parameter_set_ID),
      dataset_type(_dataset_type),
      alphabet_ID(_alphabet_id),
      read_length(_read_length),
      number_of_template_segments_minus1(_paired_end),
      max_au_data_unit_size(0),
      pos_40_bits_flag(_pos_40_bits_flag),
      qv_depth(_qv_depth),
      as_depth(_as_depth),
      class_IDs(0),
      descriptors(18),
      rgroup_IDs(0),
      multiple_alignments_flag(_multiple_alignments_flag),
      spliced_reads_flag(_spliced_reads_flag),
      multiple_signature_base(0),
      u_signature_size(nullptr),
      qv_coding_configs(0),
      crps_flag(false),
      parameter_set_crps(nullptr) {
    for (auto &i : descriptors) {
        i = util::make_unique<DescriptorConfigurationContainer>();
    }
}

ParameterSet::ParameterSet()
    : DataUnit(DataUnitType::PARAMETER_SET),
      parameter_set_ID(0),
      parent_parameter_set_ID(0),
      dataset_type(DatasetType::ALIGNED),
      alphabet_ID(genie::AlphabetID::ACGTN),
      read_length(0),
      number_of_template_segments_minus1(0),
      max_au_data_unit_size(0),
      pos_40_bits_flag(false),
      qv_depth(0),
      as_depth(0),
      class_IDs(0),
      descriptors(18),
      rgroup_IDs(0),
      multiple_alignments_flag(false),
      spliced_reads_flag(false),
      multiple_signature_base(0),
      u_signature_size(nullptr),
      qv_coding_configs(0),
      crps_flag(false),
      parameter_set_crps(nullptr) {}

// -----------------------------------------------------------------------------------------------------------------

void ParameterSet::write(util::BitWriter &writer) const {
    DataUnit::write(writer);
    writer.write(0, 10);  // reserved

    // Calculate size and write structure to tmp buffer
    std::stringstream ss;
    util::BitWriter tmp_writer(&ss);
    preWrite(tmp_writer);
    tmp_writer.flush();
    uint64_t bits = tmp_writer.getBitsWritten();
    const uint64_t TYPE_SIZE_SIZE = 8 + 10 + 22;  // data_unit_type, reserved, data_unit_size
    bits += TYPE_SIZE_SIZE;
    uint64_t bytes = bits / 8 + ((bits % 8) ? 1 : 0);

    // Now size is known, write to final destination
    writer.write(bytes, 22);
    writer.write(&ss);
}

// -----------------------------------------------------------------------------------------------------------------

void ParameterSet::preWrite(util::BitWriter &writer) const {
    writer.write(parameter_set_ID, 8);
    writer.write(parent_parameter_set_ID, 8);
    writer.write(uint8_t(dataset_type), 4);
    writer.write(uint8_t(alphabet_ID), 8);
    writer.write(read_length, 24);
    writer.write(number_of_template_segments_minus1, 2);
    writer.write(0, 6);  // reserved_2
    writer.write(max_au_data_unit_size, 29);
    writer.write(pos_40_bits_flag, 1);
    writer.write(qv_depth, 3);
    writer.write(as_depth, 3);
    writer.write(class_IDs.size(), 4);  // num_classes
    for (auto &i : class_IDs) {
        writer.write(uint8_t(i), 4);
    }
    for (auto &i : descriptors) {
        i->write(writer);
    }
    writer.write(rgroup_IDs.size(), 16);  // num_groups
    for (auto &i : rgroup_IDs) {
        for (auto &j : i) {
            writer.write(j, 8);
        }
        writer.write('\0', 8);  // NULL termination
    }
    writer.write(multiple_alignments_flag, 1);
    writer.write(spliced_reads_flag, 1);
    writer.write(multiple_signature_base, 31);
    if (u_signature_size) {
        writer.write(*u_signature_size, 6);
    }
    for (auto &i : qv_coding_configs) {
        i->write(writer);
    }
    writer.write(crps_flag, 1);
    if (parameter_set_crps) {
        parameter_set_crps->write(writer);
    }
}

// -----------------------------------------------------------------------------------------------------------------

const ParameterSetCrps &ParameterSet::getParameterSetCrps() const { return *parameter_set_crps; }

size_t ParameterSet::getNumberTemplateSegments() const { return number_of_template_segments_minus1 + 1; }

void ParameterSet::setCrps(std::unique_ptr<ParameterSetCrps> _parameter_set_crps) {
    crps_flag = true;
    parameter_set_crps = std::move(_parameter_set_crps);
}

// -----------------------------------------------------------------------------------------------------------------

void ParameterSet::addClass(genie::mpegg_rec::ClassType class_id, std::unique_ptr<QvCodingConfig> conf) {
    for (auto &a : descriptors) {
        if (a->isClassSpecific()) {
            UTILS_THROW_RUNTIME_EXCEPTION("Adding classes not allowed once class specific descriptor configs enabled");
        }
    }
    for (const auto &a : class_IDs) {
        if (class_id == a) {
            UTILS_THROW_RUNTIME_EXCEPTION("Class already added");
        }
    }
    class_IDs.push_back(class_id);
    qv_coding_configs.push_back(std::move(conf));
}

// -----------------------------------------------------------------------------------------------------------------

void ParameterSet::setDescriptor(genie::GenDesc index, std::unique_ptr<DescriptorConfigurationContainer> descriptor) {
    descriptors[uint8_t(index)] = std::move(descriptor);
}

const DescriptorConfigurationContainer *ParameterSet::getDescriptor(genie::GenDesc index) const {
    return descriptors[uint8_t(index)].get();
}

// -----------------------------------------------------------------------------------------------------------------

void ParameterSet::addGroup(std::unique_ptr<std::string> rgroup_id) { rgroup_IDs.push_back(std::move(*rgroup_id)); }

// -----------------------------------------------------------------------------------------------------------------

void ParameterSet::setMultipleSignatureBase(uint32_t _multiple_signature_base, uint8_t _U_signature_size) {
    multiple_signature_base = _multiple_signature_base;
    u_signature_size = util::make_unique<uint8_t>(_U_signature_size);
}

ParameterSet::DatasetType ParameterSet::getDatasetType() const { return dataset_type; }

uint8_t ParameterSet::getPosSize() const { return pos_40_bits_flag ? 40 : 32; }

bool ParameterSet::hasMultipleAlignments() const { return multiple_alignments_flag; }

uint32_t ParameterSet::getMultipleSignatureBase() const { return multiple_signature_base; }

uint8_t ParameterSet::getSignatureSize() const { return u_signature_size ? *u_signature_size : 0; }

uint8_t ParameterSet::getID() const { return parameter_set_ID; }

ParameterSet &ParameterSet::operator=(const ParameterSet &other) {
    if (this == &other) {
        return *this;
    }
    parameter_set_ID = other.parameter_set_ID;
    parent_parameter_set_ID = other.parent_parameter_set_ID;
    dataset_type = other.dataset_type;
    alphabet_ID = other.alphabet_ID;
    read_length = other.read_length;
    number_of_template_segments_minus1 = other.number_of_template_segments_minus1;
    max_au_data_unit_size = other.max_au_data_unit_size;
    pos_40_bits_flag = other.pos_40_bits_flag;
    qv_depth = other.qv_depth;
    as_depth = other.as_depth;
    class_IDs = other.class_IDs;
    for (const auto &d : other.descriptors) {
        descriptors.emplace_back(d->clone());
    }
    rgroup_IDs = other.rgroup_IDs;
    multiple_alignments_flag = other.multiple_alignments_flag;
    spliced_reads_flag = other.spliced_reads_flag;
    multiple_signature_base = other.multiple_signature_base;
    if (other.u_signature_size) {
        u_signature_size = util::make_unique<uint8_t>(*other.u_signature_size);
    }
    for (const auto &c : other.qv_coding_configs) {
        qv_coding_configs.emplace_back(c->clone());
    }
    crps_flag = other.crps_flag;
    parameter_set_crps = other.parameter_set_crps->clone();
    return *this;
}

ParameterSet &ParameterSet::operator=(ParameterSet &&other) noexcept {
    parameter_set_ID = other.parameter_set_ID;
    parent_parameter_set_ID = other.parent_parameter_set_ID;
    dataset_type = other.dataset_type;
    alphabet_ID = other.alphabet_ID;
    read_length = other.read_length;
    number_of_template_segments_minus1 = other.number_of_template_segments_minus1;
    max_au_data_unit_size = other.max_au_data_unit_size;
    pos_40_bits_flag = other.pos_40_bits_flag;
    qv_depth = other.qv_depth;
    as_depth = other.as_depth;
    class_IDs = std::move(other.class_IDs);
    descriptors = std::move(other.descriptors);
    rgroup_IDs = std::move(other.rgroup_IDs);
    multiple_alignments_flag = other.multiple_alignments_flag;
    spliced_reads_flag = other.spliced_reads_flag;
    multiple_signature_base = other.multiple_signature_base;
    u_signature_size = std::move(other.u_signature_size);
    qv_coding_configs = std::move(other.qv_coding_configs);
    crps_flag = other.crps_flag;
    parameter_set_crps = std::move(other.parameter_set_crps);
    return *this;
}

ParameterSet::ParameterSet(const ParameterSet &other) : DataUnit(DataUnitType::PARAMETER_SET) { *this = other; }

ParameterSet::ParameterSet(ParameterSet &&other) noexcept : DataUnit(DataUnitType::PARAMETER_SET) { *this = std::move(other); }


}  // namespace mpegg_p2
}  // namespace genie