#pragma once

struct ROUTE
{
    std::string name;
    u_short     local_port = 0;
    u_short     remote_port = 0;
    std::string remote_addr;
    boolean IsValid() { return local_port && remote_port && remote_addr.size(); }
    void deserialize(std::stringstream& s)
    {
        s >> name;
        s >> local_port;
        s >> remote_port;
        s >> remote_addr;
    }
    void serialize(std::stringstream& s) const
    {
        s << name << std::endl;
        s << local_port << std::endl;
        s << remote_port << std::endl;
        s << remote_addr << std::endl;
    }
    boolean operator == (const ROUTE& r) {
        return local_port == r.local_port && remote_port == r.remote_port && remote_addr == r.remote_addr;
    }

};

