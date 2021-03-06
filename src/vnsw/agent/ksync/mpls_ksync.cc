/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <base/logging.h>
#include <db/db_entry.h>
#include <db/db_table.h>
#include <db/db_table_partition.h>
#include <ksync/ksync_index.h>
#include <ksync/ksync_entry.h>
#include <ksync/ksync_object.h>
#include <ksync/ksync_sock.h>

#include "oper/interface.h"
#include "oper/nexthop.h"
#include "oper/mpls.h"
#include "oper/mirror_table.h"

#include "ksync/interface_ksync.h"
#include "ksync/nexthop_ksync.h"
#include "ksync/mpls_ksync.h"

#include "ksync_init.h"

void vr_mpls_req::Process(SandeshContext *context) {
    AgentSandeshContext *ioc = static_cast<AgentSandeshContext *>(context);
    ioc->MplsMsgHandler(this);
}

MplsKSyncObject *MplsKSyncObject::singleton_;

KSyncDBObject *MplsKSyncEntry::GetObject() {
    return MplsKSyncObject::GetKSyncObject();
}

MplsKSyncEntry::MplsKSyncEntry(const MplsLabel *mpls) :
    KSyncNetlinkDBEntry(kInvalidIndex), label_(mpls->GetLabel()), nh_(NULL) {
}

bool MplsKSyncEntry::IsLess(const KSyncEntry &rhs) const {
    const MplsKSyncEntry &entry = static_cast<const MplsKSyncEntry &>(rhs);

    return label_ < entry.label_;
}

std::string MplsKSyncEntry::ToString() const {
    std::stringstream s;
    NHKSyncEntry *nh = GetNH();

    if (nh) {
        s << "Mpls : " << label_ << " Index : " << GetIndex() << " NH : " 
        << nh->GetIndex();
    } else {
        s << "Mpls : " << label_ << " Index : " << GetIndex() << " NH : <null>";
    }
    return s.str();
}

bool MplsKSyncEntry::Sync(DBEntry *e) {
    bool ret = false;
    const MplsLabel *mpls = static_cast<MplsLabel *>(e);

    NHKSyncObject *nh_object = NHKSyncObject::GetKSyncObject();
    if (mpls->GetNextHop() == NULL) {
        LOG(DEBUG, "nexthop in mpls label is null");
        assert(0);
    }
    NHKSyncEntry nh(mpls->GetNextHop());
    NHKSyncEntry *old_nh = GetNH();

    nh_ = nh_object->GetReference(&nh);
    if (old_nh != GetNH()) {
        ret = true;
    }

    return ret;
};

int MplsKSyncEntry::Encode(sandesh_op::type op, char *buf, int buf_len) {
    vr_mpls_req encoder;
    int encode_len, error;
    NHKSyncEntry *nh = GetNH();

    encoder.set_h_op(op);
    encoder.set_mr_label(label_);
    encoder.set_mr_rid(0);
    encoder.set_mr_nhid(nh->GetIndex());
    encode_len = encoder.WriteBinary((uint8_t *)buf, buf_len, &error);
    return encode_len;
}

void MplsKSyncEntry::FillObjectLog(sandesh_op::type op, KSyncMplsInfo &info) {
    info.set_label(label_);
    info.set_nh(GetNH()->GetIndex());

    if (op == sandesh_op::ADD) {
        info.set_operation("ADD/CHANGE");
    } else {
        info.set_operation("DELETE");
    }
}

int MplsKSyncEntry::AddMsg(char *buf, int buf_len) {
    KSyncMplsInfo info;
    FillObjectLog(sandesh_op::ADD, info);
    KSYNC_TRACE(Mpls, info);

    return Encode(sandesh_op::ADD, buf, buf_len);
}

int MplsKSyncEntry::ChangeMsg(char *buf, int buf_len){
    KSyncMplsInfo info;
    FillObjectLog(sandesh_op::ADD, info);
    KSYNC_TRACE(Mpls, info);
 
    return Encode(sandesh_op::ADD, buf, buf_len);
}

int MplsKSyncEntry::DeleteMsg(char *buf, int buf_len) {
    KSyncMplsInfo info;
    FillObjectLog(sandesh_op::DELETE, info);
    KSYNC_TRACE(Mpls, info);
 
    return Encode(sandesh_op::DELETE, buf, buf_len);
}

KSyncEntry *MplsKSyncEntry::UnresolvedReference() {
    NHKSyncEntry *nh = GetNH();
    if (!nh->IsResolved()) {
        return nh;
    }
    return NULL;
}
