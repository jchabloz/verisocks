from email import header
from select import select
from shutil import ExecError
import socket
import sys
import io
import struct
import json


class Verisocks:

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self._connected = False

    def connect(self):
        address = (self.host, self.port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setblocking(True)
        self.sock.settimeout(120.0)
        print(f"Attempting connection to {address}")
        try:
            self.sock.connect(address)
        except all:
            raise ConnectionError(f"Could not connect to {address}")
        else:
            self._connected = True

    def close(self):
        self.sock.close()
        self.sock = None


class Message:
    """Verisocks message

    Note: The implementation for this class is partially copied from the
    example code that can be obtained from the realpython.com's tutorial on
    socket programming. It has been slightly simplified to implement a simpler,
    blocking behavior, therefore supporting only 1 connection at a time. The
    socket connection does not need to be absolutely closed with every message.
    """

    # Message state
    STATE_MSG_INIT = 0     # Initial state
    STATE_MSG_PRE_HDR = 1  # Message pre-header scanned
    STATE_MSG_HDR = 2      # Message header scanned
    STATE_MSG_CONTENT = 3  # Message content scanned
    STATE_MSG_ERROR = 4    # Error state

    def __init__(self, sock):
        """Verisocks message class constructor

        Args:
            sock (socket.socket): Socket instance
        """
        self.sock = sock
        self._rx_buffer = b""
        self._tx_buffer = b""
        self._header_len = None
        self.header = None
        self.content = None
        self._state = self.STATE_MSG_INIT

    def _read(self):
        """Reads from socket to RX buffer"""
        data = self.sock.recv(4096)
        if data:
            self._rx_buffer += data
        else:
            raise ConnectionAbortedError

    def _write(self):
        """Write TX buffer to socket"""
        if self._tx_buffer:
            sent = self.sock.send(self._tx_buffer)
            self._tx_buffer = self._tx_buffer[sent:]
            if (sent == 0):
                raise ConnectionAbortedError

    def _json_encode(self, obj, encoding="utf-8"):
        """Encode a JSON object as a bytes object

        Args:
            obj (json): JSON object
            encoding (str): Encoding to be used, default=utf-8

        Returns:
            bytes: Dumped and encoded content
        """
        return json.dumps(obj, ensure_ascii=False).encode(encoding)

    def _json_decode(self, json_bytes, encoding="utf-8"):
        """Decode JSON bytes to relevant Python object

        Args:
            json_bytes (bytes): Bytes instance
            encoding (str): Encoding, default=utf-8

        Returns:
            obj: Decoded JSON object
        """
        # Alternative, simply use
        # return json.loads(json_bytes) ??
        tiow = io.TextIOWrapper(
            io.BytesIO(json_bytes), encoding=encoding, newline=""
        )
        obj = json.load(tiow)
        tiow.close()
        return obj

    def _create_message(
        self, *, content_bytes, content_type, content_encoding
    ):
        """Create message with pre-header, header and message content"""
        json_header = {
            "content-type": content_type,
            "content-encoding": content_encoding,
            "content-length": len(content_bytes)
        }
        json_header_bytes = self._json_encode(json_header, "utf-8")
        message_pre_header = struct.pack(">H", len(json_header_bytes))
        message = message_pre_header + json_header_bytes + content_bytes
        return message

    def scan_pre_header(self):
        """Scan RX buffer for pre-header value"""
        if (self._state != self.STATE_MSG_INIT):
            raise ValueError("ERROR: Inconsistent state")
        pre_header_len = 2
        if not (len(self._rx_buffer) >= pre_header_len):
            return
        self._header_len = struct.unpack(
            ">H", self._rx_buffer[:pre_header_len]
        )[0]
        self._rx_buffer = self._rx_buffer[pre_header_len:]
        self._state = self.STATE_MSG_PRE_HDR

    def scan_header(self):
        """Scan RX buffer for header"""
        if (self._state != self.STATE_MSG_PRE_HDR):
            raise ValueError("ERROR: Inconsistent state")
        header_len = self._header_len
        if not (len(self._rx_buffer) >= header_len):
            return
        self.header = self._json_decode(
            self._rx_buffer[:header_len], "utf-8"
        )
        self._rx_buffer = self._rx_buffer[header_len:]
        header_keys = [
            "content-length",
            "content-type",
            "content-encoding"
        ]
        for k in header_keys:
            if k not in self.header:
                raise ValueError(f"Missing required header field '{k}'.")
        self._state = self.STATE_MSG_HDR

    def scan_content(self):
        """Scan RX buffer for content"""
        if (self._state != self.STATE_MSG_HDR):
            raise ValueError("ERROR: Inconsistent state")
        content_len = self.header["content-length"]
        if not (len(self._rx_buffer) >= content_len):
            return
        data = self._rx_buffer[:content_len]
        self._rx_buffer = self._rx_buffer[content_len:]

        # Process content depending on type declared in header
        if (self.header["content-type"] == "text/plain"):
            encoding = self.header["content-encoding"]
            self.content = self.decode(data, encoding)
        elif (self.header["content-type"] == "application/json"):
            encoding = self.header["content-encoding"]
            self.content = self._json_decode(data, encoding)
        elif (self.header["content-type"] == "application/octet-stream"):
            self.content = data
        else:
            raise ValueError("Value for 'content-type' not recognized")
        self._state = self.STATE_MSG_CONTENT

    def read(self, num_trials=10):
        """Proceed to read and scan message"""
        trials = 0
        while (trials < num_trials):
            self._read()
            if (self._state == self.STATE_MSG_INIT):
                self.scan_pre_header()
            if (self._state == self.STATE_MSG_PRE_HDR):
                self.scan_header()
            if (self._state == self.STATE_MSG_HDR):
                self.scan_content()
            if (self._state == self.STATE_MSG_CONTENT):
                return
            trials += 1
        self._state = self.STATE_MSG_ERROR

# EOF
