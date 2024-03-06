import socket
import logging
import struct
import json
from enum import Enum, auto
from time import sleep


class VsRxState(Enum):
    """Enumerated states for the RX state machine

    Possible values are:

        - ``RX_INIT``: Starting state
        - ``RX_PRE_HDR``: RX message pre-header has been scanned
        - ``RX_HDR``: RX message header has been scanned
        - ``RX_DONE``: RX message content has been scanned
        - ``ERROR``: Error state
    """
    RX_INIT = auto()     # Starting state
    RX_PRE_HDR = auto()  # RX message pre-header scanned
    RX_HDR = auto()      # RX message header scanned
    RX_DONE = auto()     # RX message content scanned
    ERROR = auto()       # Error state


class VerisocksError(Exception):
    """Base class for exceptions in Verisocks"""
    pass


class Verisocks:
    """Verisocks client class.

    Args:
        host (str): Server host IP address, default="127.0.0.1"
        port (int): Server port number, default=5100
        timeout (float): Socket timeout (base value),
                            in seconds (default=120)

    Note:
        For certain methods, a specific timeout value can be passed as
        argument. If a value (other than ``None``) is provided, the socket
        timeout will be modified only temprorarily and then restored to the
        timeout value provided as argument at the class instantiation.
    """

    PRE_HDR_LEN = 2  # Pre-header length in bytes
    READ_BUFFER_LEN = 4096

    def __init__(self, host="127.0.0.1", port=5100, timeout=120.0):
        """Verisocks class constructor
        """
        # Connection address and status
        self._connected = False
        self.address = (host, port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setblocking(True)
        self._timeout = None
        if timeout:
            self._timeout = timeout
            self.sock.settimeout(timeout)

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

    def connect(self, trials=10, delay=0.05):
        """Connect to server socket.

        If the client is already connected to a server socket, nothing happens.
        Otherwise, the client attempts to connect to the server as defined by
        the address and port provided to the class constructor. The method will
        apply a delay prior each connection trial and will retry a number of
        times if unsuccessful.

        Args:
            trials (int): Maximum number of connection trials
            delay (float): Delay to be applied prior each connection trial

        Raises:
            ConnectionError if all the successive connection trials are
            unsucessful
        """
        if not self._connected:
            logging.info(f"Attempting connection to {self.address}")
            trial = 0
            while trial < trials:
                try:
                    sleep(delay)
                    self.sock.connect(self.address)
                    logging.info(f"Socket connected after {trial + 1} trials")
                    self._connected = True
                    break
                except ConnectionError:
                    trial += 1
            if trial >= trials:
                raise ConnectionError(
                    f"Connection unsucessful after {trial} trials")
        else:
            logging.info("Socket already connected")

    def _read(self, timeout=None):
        """Reads from socket to RX buffer (private).

        Args:
            timeout (float): Timeout in seconds (default=None). If None, the
            base value as defined within the class constructor applies.

        Raises:
            ConnectionError if no data is received (most likely the socket is
            closed).
        """
        if timeout:
            self.sock.settimeout(timeout)
        data = self.sock.recv(self.READ_BUFFER_LEN)
        if data:
            self._rx_buffer += data
        else:
            raise ConnectionError
        if timeout:
            self.sock.settimeout(self._timeout)

    def _write(self, num_bytes, num_trials=10):
        """Write TX buffer to socket (private method)

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
        """Encode a JSON object as a bytes object (private method)

        Args:
            obj (json): JSON object
            encoding (str): Encoding to be used, default=utf-8

        Returns:
            bytes: Dumped and encoded content
        """
        return json.dumps(obj, ensure_ascii=False).encode(encoding)

    def _read_pre_header(self):
        """Parse RX buffer for pre-header value (private method).
        """
        if (self._rx_state is not VsRxState.RX_INIT):
            raise ValueError("ERROR: Inconsistent state")
        if not (len(self._rx_buffer) >= self.PRE_HDR_LEN):
            return
        self._rx_header_len = struct.unpack(
            ">H", self._rx_buffer[:self.PRE_HDR_LEN]
        )[0]
        self._rx_buffer = self._rx_buffer[self.PRE_HDR_LEN:]
        self._rx_state = VsRxState.RX_PRE_HDR

    def _read_header(self):
        """Parse RX buffer for header (private method).
        """
        if (self._rx_state is not VsRxState.RX_PRE_HDR):
            raise RuntimeError("ERROR: Inconsistent state")
        header_len = self._rx_header_len
        if not (len(self._rx_buffer) >= header_len):
            return
        logging.debug("Received message header: " +
                      self._rx_buffer[:header_len].decode("utf-8"))
        self.rx_header = json.loads(
            self._rx_buffer[:header_len].decode("utf-8"))
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

    def _read_content(self):
        """Parse RX buffer for content (private method).
        """
        if (self._rx_state is not VsRxState.RX_HDR):
            raise RuntimeError("ERROR: Inconsistent state")
        content_len = self.rx_header["content-length"]
        if not (len(self._rx_buffer) >= content_len):
            return
        data = self._rx_buffer[:content_len]
        self._rx_buffer = self._rx_buffer[content_len:]
        logging.debug("Received message content: " + repr(data))

        # Process content depending on type declared in header
        if (self.rx_header["content-type"] == "text/plain"):
            encoding = self.rx_header["content-encoding"]
            self.rx_content = data.decode(encoding)
        elif (self.rx_header["content-type"] == "application/json"):
            encoding = self.rx_header["content-encoding"]
            self.rx_content = json.loads(data.decode(encoding))
        elif (self.rx_header["content-type"] == "application/octet-stream"):
            self.rx_content = data
        else:
            raise ValueError("Value for 'content-type' not recognized")
        self._rx_state = VsRxState.RX_DONE

    def _queue_message(self, request):
        """Create message and add it to the TX queue buffer.

        Args:
            request (dict): Dictionary with the following keys: ``"content"``,
                ``"type"`` and ``"encoding"`` (unless ``"type"`` is
                ``"application/octet-stream"``, in which case the encoding is
                not applicable).
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
        logging.debug(f"Queuing message header (length {len(message_header)})\
: {repr(message_header)}")
        logging.debug(f"Queuing message content: {repr(content_bytes)}")

    def read(self, num_trials=10, timeout=None):
        """Proceed to read and scan returned TCP messages

        Args:
            num_trials (int): Maximum number of trials. Default=10.
            timeout (float): Timeout in seconds (default=None). If None, the
                base value as defined within the class constructor applies.

        Returns:
            bool: Status. ``True`` if successful, ``False`` if error.
        """
        if self._rx_expected:
            if not self._connected:
                self.connect()
            if not self._rx_buffer:
                self._read(timeout)
            trials = 0
            while (trials < num_trials):
                if (self._rx_state is VsRxState.RX_INIT):
                    self._read_pre_header()
                if (self._rx_state is VsRxState.RX_PRE_HDR):
                    self._read_header()
                if (self._rx_state is VsRxState.RX_HDR):
                    self._read_content()
                if (self._rx_state is VsRxState.RX_DONE):
                    self._rx_expected -= 1
                    self._rx_state = VsRxState.RX_INIT
                    logging.debug(f"Read procedure successful. \
Still {self._rx_expected} messages expected.")
                    return True
                self._read(timeout)
                trials += 1
            self._rx_state = VsRxState.ERROR
            logging.error("Read procedure unsuccessful")
            return False
        else:
            logging.warning("No expected message. Cancelling read procedure.")
            return False

    def write(self, all=True):
        """Writes/sends the current content of the TX buffer to the socket.

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
            logging.warning("TX buffer is empty. No message to transmit.")

    def send(self, **cmd):
        """Sends a message with a JSON content.

        This command will accept and use any command content defined as keyword
        arguments (e.g. ``cmd="get"``, ``sel="sim_info"``). This content will
        directly be used as the sent TCP command content.

        Keyword Arguments:
            timeout (float): Socket timeout configuration value in seconds.
                If None (default), the class instance default value is used.

        Returns:
            JSON object: Content of returned message.
        """

        if "timeout" in cmd:
            timeout = cmd.pop("timeout")
        else:
            timeout = None

        self._queue_message({
            "type": "application/json",
            "encoding": "utf-8",
            "content": cmd
        })
        self.write()

        if (self.read(10, timeout)):
            if self.rx_content["type"] == "error":
                raise VerisocksError(self.rx_content["value"])
            return self.rx_content
        else:
            return None

    def send_cmd(self, command, **kwargs):
        """Sends a command. Equivalent to :code:`send(command=command, ...)`.

        Args:
            command (str): Command name (e.g. ``"get"``)
            **kwargs: Command keyword arguments (e.g. ``sel="sim_info"``)

        Keyword Arguments:
            timeout (float): Socket timeout configuration value in seconds.
                If None (default), the class instance default value is used.

        Returns:
            JSON object: Content of returned message
        """
        return self.send(command=command, **kwargs)

    def run(self, cb, **kwargs):
        """Sends a :keyword:`run <sec_tcp_cmd_run>` command request to the
        Verisocks server.

        Equivalent to :code:`send_cmd("run", cb=cb, ...)`. This command gives
        the focus back to the simulator and lets it run until the specified
        callback condition is met (see arguments).

        Args:
            cb (str): Callback type.

        Keyword Arguments:
            timeout (float): Socket timeout configuration value in seconds.
                If None (default), the class instance default value is used.

        Returns:
            JSON object: Content of returned message

        Note:
            The parameter `cb` can be either:

            * ``"for_time"``: run for a given amount of time
            * ``"until_time"``: run until a specified time
            * ``"until_change"``: run until a specific value changes
            * ``"to_next"``: run until the beginning of the next time step

            If `cb` is ``"for_time"`` or ``"until_time"``, the following
            keyword arguments are further expected:

            * **time** (float): Time value
            * **time_unit** (str): Time unit (s, ms, us, ns, ps or fs). Be
              aware that depending on the simulator time resolution, the
              provided time value may be truncated.

            If `cb` is ``"until_change"``, the following keyword argument(s)
            are further expected:

            * **path** (str): Path to verilog object used for the callback
            * **value** (number): Condition on the verilog object's
              value for the callback to be executed. This argument is not
              required if the path corresponds to a named event.

            If `cb` is ``"to_next"``, no further keyword argument is required.
        """

        return self.send(command="run", cb=cb, **kwargs)

    def set(self, path, **kwargs):
        """Sends a :keyword:`set <sec_tcp_cmd_set>` command request to the
        Verisocks server.

        Equivalent to :code:`send_cmd("set", path=path, **kwargs)`. This
        commands sets the value of a verilog object as defined by its path.

        Args:
            path (str): Path to the verilog object.

        Keyword Arguments:
            value (int, list): Value to be set. If the path corresponds to a
                verilog named event, this argument is not required. If the path
                corresponds to a verilog memory array, this argument needs to
                be provided as a list of the same length.
            timeout (float): Socket timeout configuration value in seconds.
                If None (default), the class instance default value is used.

        Returns:
            JSON object: Content of returned message
        """
        return self.send(command="set", path=path, **kwargs)

    def info(self, value):
        """Sends an :keyword:`info <sec_tcp_cmd_info>` command to the Verisocks
        server.

        This is a shortcut function, which is equivalent to
        :code:`send_cmd("info", ...)`. This command is used to send any text to
        the Verisocks server, which will then be streamed out to the VPI
        standard output.

        Args:
            value (str): Text to be sent to the VPI stdout

        Returns:
            JSON object: Content of returned message
        """
        return self.send(command="info", value=value)

    def get(self, sel, path=""):
        """Sends a :keyword:`get <sec_tcp_cmd_get>` command request to the
        Verisocks server.

        Equivalent to :code:`send_cmd("get", ...)`. This commands can be
        used to obtain different pieces of information from the Verisocks
        server.

        Args:
            sel (str): Selects which is the returned information.
            path (str): If `sel` is ``"value"`` or ``"type"``, path to the
                desired verilog object for which the value or type is to be
                returned.

        Note:
            The argument `sel` can take the following values:

            * ``"sim_info"``: Gets the simulator information,
              returned with the keywords :code:`"product"` and
              :code:`"version"`.
            * ``"sim_time"``: Gets the simulator current time, returned
              with the keywords :code:`"time"`, in seconds.
            * ``"value"``: Gets the value for a verilog object.
            * ``"type"``: Gets the VPI type value for a verilog object.

        Returns:
            JSON object: Content of returned message
        """
        return self.send(command="get", sel=sel, path=path)

    def finish(self, timeout=None):
        """Sends a :keyword:`finish <sec_tcp_cmd_finish>` command to the
        Verisocks server that terminates the simulation (and therefore also
        closes the Verisocks server itself). The connection is closed as well
        by the function as a clean-up.
        """
        retval = self.send(command="finish", timeout=timeout)
        self.close()
        return retval

    def stop(self, timeout=None):
        """Sends a :keyword:`stop <sec_tcp_cmd_stop>` command to the Verisocks
        server.

        The ``"stop"`` command stops the simulation. The Verisocks server
        socket is not closed, but the simulation has to be restarted for any
        new request to be processed.

        Args:
            timeout (int, None): Timeout in seconds to apply for the execution
                of the command.
        """
        return self.send(command="stop", timeout=timeout)

    def exit(self):
        """Sends an :keyword:`exit <sec_tcp_cmd_exit>` command to the Verisocks
        server that gives back control to the simulator and closes the
        Verisocks server socket. The simulation runs to its end without having
        the possibility to take the control back from the simulator anymore.
        The connection is closed as well by the function."""
        retval = self.send(command="exit")
        self.close()
        return retval

    def close(self):
        """Close socket connection if still open."""
        if self._connected:
            logging.info("Closing socket connection")
            self.sock.close()
            self.sock = None
            self._connected = False

    def flush(self):
        """Flush RX and TX buffers."""
        logging.debug("Flushing RX and TX buffers")
        self._rx_buffer = b""
        self._tx_buffer = b""
        # self._rx_expected = 0
        self._tx_msg_len = []

    def __enter__(self):
        """Context manager - Entry function"""
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, exc_tb):
        """Context manager - Exit function"""
        self.close()
        return False

# EOF
