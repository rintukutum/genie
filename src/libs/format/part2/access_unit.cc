#include "access_unit.h"
#include "ureads-encoder/exceptions.h"
#include "util/bitwriter.h"


#include <gabac/data-block.h>
#include <sstream>

// -----------------------------------------------------------------------------------------------------------------

namespace format {
AccessUnit::AccessUnit(uint32_t _access_unit_ID, uint8_t _parameter_set_ID, AuType _au_type, uint32_t _reads_count,
                       DatasetType dataset_type, uint8_t posSize, uint8_t signatureSize,
                       uint32_t multiple_signature_base)
    : DataUnit(DataUnitType::ACCESS_UNIT),
      reserved(0),
      data_unit_size(0),
      access_unit_ID(_access_unit_ID),
      num_blocks(0),
      parameter_set_ID(_parameter_set_ID),
      au_type(_au_type),
      reads_count(_reads_count),
      mm_cfg(nullptr),
      ref_cfg(nullptr),
      au_Type_U_Cfg(nullptr),
      signature_config(nullptr),
      blocks(0) {
    if (au_type == AuType::N_TYPE_AU || au_type == AuType::M_TYPE_AU) {
        mm_cfg = make_unique<MmCfg>();
    }
    if (dataset_type == DatasetType::REFERENCE) {
        ref_cfg = make_unique<RefCfg>(posSize);
    }
    if (au_type != AuType::U_TYPE_AU) {
        au_Type_U_Cfg = make_unique<AuTypeCfg>(posSize);
    } else {
        if (multiple_signature_base != 0) {
            signature_config = make_unique<SignatureCfg>(0, signatureSize);
        }
    }
}

void AccessUnit::setMmCfg(std::unique_ptr<MmCfg> cfg) {
    if (!mm_cfg) {
        GENIE_THROW_RUNTIME_EXCEPTION("MmCfg not valid for this access unit");
    }
    mm_cfg = std::move(cfg);
}

void AccessUnit::setRefCfg(std::unique_ptr<RefCfg> cfg) {
    if (!ref_cfg) {
        GENIE_THROW_RUNTIME_EXCEPTION("RefCfg not valid for this access unit");
    }
    ref_cfg = std::move(cfg);
}

void AccessUnit::setAuTypeCfg(std::unique_ptr<AuTypeCfg> cfg) {
    if (!au_Type_U_Cfg) {
        GENIE_THROW_RUNTIME_EXCEPTION("au_type_u_cfg not valid for this access unit");
    }
    au_Type_U_Cfg = std::move(cfg);
}

void AccessUnit::setSignatureCfg(std::unique_ptr<SignatureCfg> cfg) {
    if (!signature_config) {
        GENIE_THROW_RUNTIME_EXCEPTION("signature config not valid for this access unit");
    }
    signature_config = std::move(cfg);
}

// -----------------------------------------------------------------------------------------------------------------

void AccessUnit::write(util::BitWriter *writer) const {
    DataUnit::write(writer);
    writer->write(reserved, 3);

    // Calculate size and write structure to tmp buffer
    std::stringstream ss;
    util::BitWriter tmp_writer(&ss);
    preWrite(&tmp_writer);
    tmp_writer.flush();
    uint64_t bits = tmp_writer.getBitsWritten();
    for (auto &i : blocks) {
        bits += i->getTotalSize() * uint64_t(8);
    }
    const uint64_t TYPE_SIZE_SIZE = 8 + 3 + 29;  // data_unit_type, reserved, data_unit_size
    bits += TYPE_SIZE_SIZE;
    const uint64_t bytes = bits / 8;

    // Now size is known, write to final destination
    writer->write(bytes, 29);
    writer->write(&ss);
    for (auto &i : blocks) {
        i->write(writer);
    }
}

// -----------------------------------------------------------------------------------------------------------------

void AccessUnit::preWrite(util::BitWriter *writer) const {
    writer->write(access_unit_ID, 32);
    writer->write(num_blocks, 8);
    writer->write(parameter_set_ID, 8);
    writer->write(uint8_t(au_type), 4);
    writer->write(reads_count, 32);
    if (mm_cfg) {
        mm_cfg->write(writer);
    }
    if (ref_cfg) {
        ref_cfg->write(writer);
    }
    if (au_Type_U_Cfg) {
        au_Type_U_Cfg->write(writer);
    }
    if (signature_config) {
        signature_config->write(writer);
    }
}

// -----------------------------------------------------------------------------------------------------------------

void AccessUnit::addBlock(std::unique_ptr<Block> block) {
    data_unit_size += block->getTotalSize();
    ++num_blocks;
    blocks.push_back(std::move(block));
}
}  // namespace format