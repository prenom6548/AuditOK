import socket
import time

def open_socket(hostname, port):
    try:
        host, port = hostname.split(":", 1)
    except ValueError:
        host = hostname

    s = None
    try:
        ai = socket.getaddrinfo(host, port, socket.AF_UNSPEC,
                                socket.SOCK_STREAM)
    except (socket.gaierror, socket.error):
        ai = []

    for res in ai:
        af, socktype, proto, canonname, sa = res
        try:
            s = socket.socket(af, socktype, proto)
            s.settimeout(0.5)
        except socket.error:
            s = None
            continue
        try:
            s.connect(sa)
        except socket.error:
            s.close()
            s = None
            continue
        break
    return s

class LpdServer:
    def __init__(self, hostname):
        self.hostname = hostname
        self.max_lpt_com = 8
        self.stop = False

    def probe_queue(self,name, result):
        s = open_socket(self.hostname, 515)
        if not s:
            return None
        print(name)
        
        try:
            s.send(('\2%s\n' % name).encode('UTF-8'))  # cmd send job to queue
            data = s.recv(1024).decode('UTF-8')  # receive status
            print(repr(data))
        except socket.error as msg:
            print(msg)
            try:
                s.close ()
            except:
                pass

            return False

        if len(data)>0 and ord(data[0])==0:
            try:
                s.send(b'\1\n')  # abort job again
                s.close ()
            except:
                pass

            result.append(name)
            return True

        try:
            s.close()
        except:
            pass

        return False

    def get_possible_queue_names (self):
        candidate = ["PASSTHRU", "ps", "lp", "PORT1", ""]
        for nr in range (self.max_lpt_com):
            candidate.extend (["LPT%d" % nr,
                               "LPT%d_PASSTHRU" % nr,
                               "COM%d" % nr,
                               "COM%d_PASSTHRU" % nr])
        for nr in range (50):
            candidate.append ("pr%d" % nr)

        return candidate

    def destroy(self):
        print ("LpdServer exiting: destroy called")
        self.stop = True

    def probe(self):
        result = []
        for name in self.get_possible_queue_names ():

            if self.stop:
                break

            found = self.probe_queue(name, result)
            if found is None:
                # Couldn't even connect.
                break

            if not found and name.startswith ("pr"):
                break
            time.sleep(0.1) # avoid DOS and following counter measures 

        return result
    

def _probe_lpd (hostname):
        print ("lpd: trying")
        lpd = LpdServer (hostname)
        for name in lpd.get_possible_queue_names ():
            print("---------------------------------------------------------")
            print(name)

            found = lpd.probe_queue (name, [])
            print(found)
            if found is None:
                # Couldn't even connect.
                print ("lpd: couldn't connect")
                break

            if found:
                if len(name) ==0:
                    modified_name = hostname+'(null)'
                else:
                    modified_name = hostname+"("+name+')'
                uri = "lpd://%s/%s" % (hostname, name)
                #self._new_device(uri, self.hostname)
                print(uri, modified_name)

            if not found and name.startswith ("pr"):
                break

            time.sleep(0.1) # avoid DOS and following counter measures 

_probe_lpd("10.41.120.170")