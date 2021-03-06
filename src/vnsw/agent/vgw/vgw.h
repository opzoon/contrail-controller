/*
 * Copyright (c) 2013 Juniper Networks, Inc. All rights reserved.
 */
#ifndef vnsw_agent_vgw_h
#define vnsw_agent_vgw_h

// Simple Virtual Gateway operational class
// Creates the interface, route and nexthop for virtual-gateway
class VirtualGateway {
public:
    VirtualGateway(Agent *agent);
    ~VirtualGateway() {};

    void Init();
    void Shutdown();
    void InterfaceNotify(DBTablePartBase *partition, DBEntryBase *entry);
    void CreateVrf();
    void CreateInterfaces();
    void RegisterDBClients();
private:
    // Cached entries
    Agent *agent_;
    VirtualGatewayConfig *vgw_config_;

    // Listener to interface oper-db. Waits for interface creation
    // before adding route and nexthop
    DBTableBase::ListenerId lid_;

    uint32_t label_;   // Label for vgw interface

    DISALLOW_COPY_AND_ASSIGN(VirtualGateway);
};

#endif //vnsw_agent_vgw_h
