#include <iostream>
#include "cd3.pb.h"
#include "zmq.hpp"
#include <boost/date_time.hpp>
#include <string>

using namespace boost::posix_time;
using namespace boost::gregorian;

DateTime *ptime_to_DataTime(const ptime &pt) {
    DateTime *dt = new DateTime;
    dt->set_day(pt.date().day());
    dt->set_month(pt.date().month());
    dt->set_year(pt.date().year());

    time_duration td = pt.time_of_day();
    dt->set_hour(td.hours());
    dt->set_minute(td.minutes());
    dt->set_second(td.seconds());
    assert(dt->IsInitialized());
    return dt;
}

struct RPCNodeWrapper {

    RPCNodeWrapper(const std::string &id, zmq::socket_t &socket) : id(id), socket(socket) {

    }

    bool init(ptime start, ptime stop, int dt) {

        DateTime *dt_start = ptime_to_DataTime(start);
        DateTime *dt_stop = ptime_to_DataTime(stop);

        NInit *init = new NInit;
        init->set_allocated_start(dt_start);
        init->set_allocated_stop(dt_stop);
        init->set_dt(dt);
        assert(init->IsInitialized());

        Call c;
        c.set_type(Call::N_INIT);
        c.set_allocated_n_init(init);
        assert(c.IsInitialized());

        std::string buf = c.SerializeAsString();
        zmq::message_t msg((void*) buf.data(), buf.size(), 0);

        socket.send(msg);
        zmq::message_t reply;
        socket.recv(&reply);

        return false;
    }

    void deinit() {
        std::cout << "unimplemented" << std::endl;
    }

    void set_parameter(const std::string &_name, double value) {
        Value *v = new Value;
        v->set_type(DOUBLE);
        v->set_double_(value);
        assert(v->IsInitialized());

        Parameter *p = new Parameter;
        p->set_allocated_value(v);
        std::string *name = new std::string(_name);
        p->set_allocated_name(name);
        assert(p->IsInitialized());

        NSetParameter *sp = new NSetParameter;
        sp->set_allocated_parameter(p);
        assert(sp->IsInitialized());

        Call c;
        c.set_type(Call::N_SET_PARAMETER);
        c.set_allocated_n_set_parameter(sp);
        assert(c.IsInitialized());

        std::string buf = c.SerializeAsString();
        zmq::message_t req((void*)buf.data(), buf.length(), 0);
        socket.send(req);

        zmq::message_t reply;
        socket.recv(&reply);
    }

    double get_parameter(const std::string &name) const {
        std::string *allocated_name = new std::string(name);

        NGetParameter *gp = new NGetParameter;
        gp->set_allocated_name(allocated_name);
        assert(gp->IsInitialized());

        Call c;
        c.set_type(Call::N_GET_PARAMETER);
        c.set_allocated_n_get_parameter(gp);
        assert(c.IsInitialized());

        std::string buf = c.SerializeAsString();
        zmq::message_t req((void*) buf.data(), buf.length(), 0);

        socket.send(req);
        zmq::message_t resp;
        socket.recv(&resp);

        Response r;
        bool parsed = r.ParseFromArray(resp.data(), resp.size());
        assert(parsed);
        return r.n_get_parameter().value().double_();
    }

    int f(ptime time, int dt) {
        NF *f = new NF;
        f->set_dt(dt);
        f->set_allocated_current_time(ptime_to_DataTime(time));
        assert(f->IsInitialized());

        Call c;
        c.set_type(Call::N_F);
        c.set_allocated_n_f(f);
        assert(c.IsInitialized());

        std::string buf = c.SerializeAsString();
        zmq::message_t req((void*) buf.data(), buf.length(), 0);

        socket.send(req);
        zmq::message_t resp;
        socket.recv(&resp);

        Response r;
        r.ParseFromArray(resp.data(), resp.size());
        assert(r.type() == Call::N_F);
        return r.n_f();
    }

    std::string id;
    zmq::socket_t &socket;
};

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    zmq::context_t ctx(1);
    zmq::socket_t socket(ctx, ZMQ_REQ);
    socket.connect("ipc:///tmp/cd3_socket");

    RPCNodeWrapper node("sewer", socket);

    node.set_parameter("xy", 1.0);
    node.set_parameter("length", 10.0);
    node.set_parameter("answer", 42.0);
    assert(node.get_parameter("answer") == 42.0);
    assert(node.get_parameter("length") == 10.0);
    assert(node.get_parameter("xy") == 1.0);

    ptime start(date(2000, 1, 1));
    ptime stop(date(2000, 1, 2));
    node.init(start, stop, 5*60);

    node.f(start, 5*60);

    node.deinit();
}

