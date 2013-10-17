from cd3_pb2 import *
import zmq

parameters = {}
ctx = zmq.Context()
socket = zmq.Socket(ctx, zmq.REP)
dt = 0

def handle_set_parameter(call):
	assert(call.type == Call.N_SET_PARAMETER)
	p = call.n_set_parameter.parameter
	parameters[p.name] = p.value.double
	socket.send_string('')

def handle_get_parameter(call):
	print call
	print 'gaga', call.n_get_parameter.name
	v = parameters[call.n_get_parameter.name]
	nr = Response()
	nr.type = Call.N_GET_PARAMETER
	pr = nr.n_get_parameter
	value = pr.value
	value.type = DOUBLE
	value.double = v

	socket.send(nr.SerializeToString())

def handle_init(call):
	init = call.n_init
	start = init.start
	stop = init.stop
	print dt
	dt = init.dt
	print dt
	r = Response()
	r.type = Call.N_INIT
	r.n_init = True
	socket.send(r.SerializeToString())
	pass

def handle_deinit(call):
	print 'unimplemented'
	socket.send_string('')
	pass

def handle_f(call):
	nf = call.n_f
	ct = nf.current_time
	dt = nf.dt

	resp = Response()
	resp.type = Call.N_F
	resp.n_f = dt
	socket.send(resp.SerializeToString())
	pass

def handle_contains(call):
	print 'unimplemented'
	socket.send_string('')
	pass

def handle_create_node(call):
	print 'unimplemented'
	socket.send_string('')
	pass

def handle_get_registered_names(call):
	print 'unimplemented'
	socket.send_string('')
	pass

def main():
	socket.bind("ipc:///tmp/cd3_socket")
	print 'binding'
	
	handlers = { 
		Call.N_SET_PARAMETER : handle_set_parameter,
		Call.N_GET_PARAMETER : handle_get_parameter,
		Call.N_INIT : handle_init,
		Call.N_DE_INIT : handle_deinit,
		Call.N_F : handle_f,
		Call.NR_CONTAINS : handle_contains,
		Call.NR_CREATE_NODE : handle_create_node,
		Call.NR_GET_REGISTERED_NAMES : handle_get_registered_names,
	}

	while True:
		msg = socket.recv()
		c = Call()
		c.ParseFromString(msg)
		
		print 'message received'

		handlers[c.type](c)

if __name__ == '__main__':
	main()
