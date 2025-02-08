import time

import pyvisa

IP_ADDRESS = "10.59.73.20"


rm = pyvisa.ResourceManager("@py")
dev = rm.open_resource(f"TCPIP::{IP_ADDRESS}::23::SOCKET")
dev.read_termination = "\n"
print(dev.query("*IDN?"))
