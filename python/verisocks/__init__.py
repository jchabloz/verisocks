import socket
import logging
import io
import struct
import json
from enum import Enum, auto


class VsRxState(Enum):
    """Enum"""
    RX_INIT = auto()     # Starting state
    RX_PRE_HDR = auto()  # RX message pre-header scanned
    RX_HDR = auto()      # RX message header scanned
    RX_DONE = auto()  # RX message content scanned
    ERROR = auto()       # Error state


class Verisocks:
    """Verisocks class

    Note: The implementation for this class is partially copied from the
    example code that can be obtained from the realpython.com's tutorial on
    socket programming. It has been slightly simplified to implement a simpler,
    blocking behavior, therefore supporting only 1 connection at a time. The
    socket connection does not need to be absolutely closed with every message.
    """

    PRE_HDR_LEN = 2  # Pre-header length

    def __init__(self, host, port):
        """Verisocks class constructor

        Args:
            host (str): Host IP address
            port (int): Port number
        """
        # Connection address and status
        self._connected = False
        self.address = (host, port)
        self.sock = None

        # RX variables
        self._rx_buffer = b""
        self._rx_header_len = None
        self._rx_state = VsRxState.RX_INIT
        self._rx_expected = 0
        self.rx_header = None
        self.rx_content = None

        # TX variables
        self._tx_buffer = b""
        self._tx_msg_len = []

        # Set up logging
        fmt = '%(levelname)s: %(asctime)s - %(message)s'
        logging.basicConfig(level=logging.DEBUG, format=fmt)

    def connect(self, timeout=120.0):
        """Connect socket

        Args:
            timeout (float): Timeout in seconds (default=120.0)
        """
        if not self.sock:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.setblocking(True)
            self.sock.settimeout(timeout)
        logging.info(f"Attempting connection to {self.address}")
        try:
            self.sock.connect(self.address)
        except Exception:
            logging.exception("Exception occurred")
        else:
            logging.info("Socket connected")
            self._connected = True

    def close(self):
        logging.info("Closing socket connection")
        self.sock.close()
        self.sock = None
        self._connected = False

    def flush(self):
        self._rx_buffer = b""
        self._tx_buffer = b""
        self._rx_expected = 0
        self._tx_msg_len = []

    def _read(self):
        """Reads from socket to RX buffer (private)

        Raises:
            ConnectionError if no data is received (most likely the socket is
            closed).
        """
        data = self.sock.recv(4096)
        if data:
            self._rx_buffer += data
        else:
            raise ConnectionError

    def _write(self, num_bytes, num_trials=10):
        """Write TX buffer to socket (private)

        Args:
            num_bytes (int): Number of bytes to send

        Raises:
            ConnectionError if no data is written (most likely the socket is
            closed).
        """
        sent = 0
        trials = 0
        while ((sent < num_bytes) and (trials < num_trials)):
            sent += self.sock.send(self._tx_buffer[sent:num_bytes])
            if (sent == 0):
                self._connected = False
                raise ConnectionError
            trials += 1

        if (sent == num_bytes):
            self._tx_buffer = self._tx_buffer[sent:]
            logging.debug(
                f"Sent {num_bytes} bytes on socket in {trials} trial(s)")
        else:
            logging.error("Did not succeed to write message to socket")
            self.close()

    def _json_encode(self, obj, encoding="utf-8"):
        """Encode a JSON object as a bytes object (private)

        Args:
            obj (json): JSON object
            encoding (str): Encoding to be used, default=utf-8

        Returns:
            bytes: Dumped and encoded content
        """
        return json.dumps(obj, ensure_ascii=False).encode(encoding)

    def _json_decode(self, json_bytes, encoding="utf-8"):
        """Decode JSON bytes to relevant Python object (private)

        Args:
            json_bytes (bytes): Bytes instance
            encoding (str): Encoding, default=utf-8

        Returns:
            obj: Decoded JSON object
        """
        # Alternative, simply use
        # return json.loads(json_bytes.decode(encoding)) ??
        tiow = io.TextIOWrapper(
            io.BytesIO(json_bytes), encoding=encoding, newline=""
        )
        obj = json.load(tiow)
        tiow.close()
        return obj

    def read_pre_header(self):
        """Parse RX buffer for pre-header value"""
        if (self._rx_state is not VsRxState.RX_INIT):
            raise ValueError("ERROR: Inconsistent state")
        if not (len(self._rx_buffer) >= self.PRE_HDR_LEN):
            return
        self._rx_header_len = struct.unpack(
            ">H", self._rx_buffer[:self.PRE_HDR_LEN]
        )[0]
        self._rx_buffer = self._rx_buffer[self.PRE_HDR_LEN:]
        self._rx_state = VsRxState.RX_PRE_HDR

    def read_header(self):
        """Parse RX buffer for header"""
        if (self._rx_state is not VsRxState.RX_PRE_HDR):
            raise RuntimeError("ERROR: Inconsistent state")
        header_len = self._rx_header_len
        if not (len(self._rx_buffer) >= header_len):
            return
        self.rx_header = self._json_decode(
            self._rx_buffer[:header_len], "utf-8"
        )
        self._rx_buffer = self._rx_buffer[header_len:]
        header_keys = [
            "content-length",
            "content-type",
            "content-encoding"
        ]
        for k in header_keys:
            if k not in self.rx_header:
                raise ValueError(f"Missing required header field '{k}'.")
        self._rx_state = VsRxState.RX_HDR

    def read_content(self):
        """Parse RX buffer for content"""
        if (self._rx_state is not VsRxState.RX_HDR):
            raise RuntimeError("ERROR: Inconsistent state")
        content_len = self.rx_header["content-length"]
        if not (len(self._rx_buffer) >= content_len):
            return
        data = self._rx_buffer[:content_len]
        self._rx_buffer = self._rx_buffer[content_len:]

        # Process content depending on type declared in header
        if (self.rx_header["content-type"] == "text/plain"):
            encoding = self.rx_header["content-encoding"]
            self.rx_content = self.decode(data, encoding)
        elif (self.rx_header["content-type"] == "application/json"):
            encoding = self.rx_header["content-encoding"]
            self.rx_content = self._json_decode(data, encoding)
        elif (self.rx_header["content-type"] == "application/octet-stream"):
            self.rx_content = data
        else:
            raise ValueError("Value for 'content-type' not recognized")
        self._rx_state = VsRxState.RX_DONE

    def queue_message(self, request):
        """Create message and add it to the TX buffer

        Args:
            request (dict): Dictionnary with the following keys: "content",
            "type" and "encoding" (unless "type" is "application/octet-stream).
        """

        content = request["content"]
        content_type = request["type"]

        # Prepare content
        if (content_type == "text/plain"):
            content_encoding = request["encoding"]
            content_bytes = bytes(content, encoding=content_encoding)
        elif (content_type == "application/json"):
            content_encoding = request["encoding"]
            content_bytes = self._json_encode(content, content_encoding)
        elif (content_type == "application/octet-stream"):
            content_bytes = bytes(content)
        else:
            raise ValueError(
                f"Value for 'content_type' {content_type} not recognized")

        # Create header
        if content_type == "application/octet-stream":
            json_header = {
                "content-type": content_type,
                "content-length": len(content_bytes)
            }
        else:
            json_header = {
                "content-type": content_type,
                "content-encoding": content_encoding,
                "content-length": len(content_bytes)
            }
        message_header = self._json_encode(json_header, "utf-8")

        # Adjust pre-header
        message_pre_header = struct.pack(">H", len(message_header))

        # Compose full message and add it to the buffer
        message = message_pre_header + message_header + content_bytes
        self._tx_buffer += message
        self._tx_msg_len.append(len(message))

    def read(self, num_trials=10):
        """Proceed to read and scan returned message"""

        if self._rx_expected:
            if not self._connected:
                self.connect()
            if not self._rx_buffer:
                self._read()
            trials = 0
            while (trials < num_trials):
                if (self._rx_state is VsRxState.RX_INIT):
                    self.read_pre_header()
                if (self._rx_state is VsRxState.RX_PRE_HDR):
                    self.read_header()
                if (self._rx_state is VsRxState.RX_HDR):
                    self.read_content()
                if (self._rx_state is VsRxState.RX_DONE):
                    self._rx_expected -= 1
                    self._rx_state = VsRxState.RX_INIT
                    return True
                self._read()
                trials += 1
            self._rx_state = VsRxState.ERROR
            return False

    def write(self, all=True):
        """Writes/sends content of the TX buffer to the socket

        Args:
            all (bool): If True (default), sends all queued message. Otherwise,
            it only sends the oldest queued message in the buffer.
        """
        if self._tx_buffer:  # Check that the TX buffer is not empty
            if not self._connected:
                self.connect()
            if all:
                self._write(len(self._tx_buffer))
                self._rx_expected += len(self._tx_msg_len)
                self._tx_msg_len.clear()
            else:
                self._write(self._tx_msg_len.pop(0))
                self._rx_expected += 1
        else:
            logging.warning("TX buffer is empty. No message to transmit")

# EOF
