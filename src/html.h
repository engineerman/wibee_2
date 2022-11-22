
#ifndef __HTTML___
#define __HTTML___

#include "pgmspace.h"

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<!DOCTYPE html>

<html>
  <head>
    <style>
      textarea {
        overflow-y: scroll;
        height: 500px;
        width: 500px;
        resize: none; /* Remove this if you want the user to resize the textarea */
      }

      .column {
        float: left;
        width: 50%;
      }

      /* Clear floats after the columns */
      .row:after {
        content: "";
        display: table;
        clear: both;
      }
    </style>

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>
  </head>

  <body>
    <h1 id="title">Wibee Uart WiFi Bridge</h1>

    <input
      placeholder="HostAddress"
      id="txtHostIP"
      name="txtHostIP"
      type="text"
      class="validate"
      autocomplete="on"
    />
    <input
      type="button"
      id="btnConnect"
      onClick="connectToHost()"
      value="Connect"
    />

    <br />
    <br />
    <select name="baudrate" id="ddBaudrate">
      <option value="9600">9600</option>
      <option value="115200">115200</option>
      <option value="460800">460800</option>
    </select>
    <input type="button" id="btnSetBaudrate" onClick="setBaud()" value="Set" />

    <br />
    <br />

    <input placeholder="TX" id="txtUART1Cmd" type="text" class="validate" />
    <input
      type="button"
      id="btnSend"
      onClick="sendUARTTx()"
      value="Send UART1"
    />

    <br />
    <div style="overflow-x: auto">
      <table id="tLayout" style="width: 40%">
        <tr>
          <td style="width: 10%">
            <label for="textarea1">UART1</label>
            <br />
            <textarea id="textarea1"></textarea>
          </td>
          <td style="width: 10%">
            <label for="textarea1">UART2</label>
            <br />
            <textarea id="textarea2"></textarea>
          </td>
        </tr>
      </table>
    </div>

    <input
      type="checkbox"
      id="cbEnAutoScroll"
      name="enAutoScroll"
      value="Enable Scroll"
      checked="true"
      onClick="enAutoScrollClicked()"
    />
    <label>Auto Scroll</label><br />

    <input
      type="button"
      id="btnClear"
      onClick="clear_console()"
      value="Clear"
    />

    <br />
    <br />

    <input placeholder="Command" id="txtCommand" type="text" class="validate" />
    <input
      type="button"
      id="btnSendCmd"
      onClick="sendTextCommand()"
      value="Cmd"
    />

    <br />
    <br />

    <input type="button" id="btnStatus" onClick="getStatus()" value="Status" />

    <br />
    <br />
    <input
      type="button"
      id="btnDelLog"
      onClick="onDelLog()"
      value="Delete Log"
    />
    <br />
    <br />
    <input type="button" id="btnSetDate" onClick="setDate()" value="Set Date" />
    <br />
    <br />

    <input
      type="checkbox"
      id="cbEnLog"
      name="enLog"
      value="Enable Log"
      onClick="enLogClicked()"
    />
    <label> Enable SD Card Log</label><br />

    <br />
    <a href="/log" download>Download Log File</a>
  </body>
  <script type="text/javascript">
    var _version;
    var _socketId;

    var _autoScroll = true;

    console.log(window.location.hostname);

    window.addEventListener("load", (event) => {
      console.log("page is fully loaded");

      if (window.location.hostname.startsWith("127.0.0.1"))
        document.getElementById("txtHostIP").value = "192.168.1.30";
      else
        document.getElementById("txtHostIP").value = window.location.hostname;
    });

    document
      .getElementById("txtUART1Cmd")
      .addEventListener("keyup", function (event) {
        event.preventDefault();
        if (event.keyCode === 13) {
          document.getElementById("btnSend").click();
        }
      });

    document
      .getElementById("txtCommand")
      .addEventListener("keyup", function (event) {
        event.preventDefault();
        if (event.keyCode === 13) {
          document.getElementById("btnSendCmd").click();
        }
      });

    var ws = null;

    function sendUARTTx() {
      var cmd = document.getElementById("txtUART1Cmd").value;

      sendCommand("tx(" + cmd + ")\n");
    }

    function sendTextCommand() {
      var cmd = document.getElementById("txtCommand").value;

      sendCommand(cmd);
    }

    function sendCommand(cmd) {
      if (ws != null && ws.readyState !== WebSocket.CLOSED) {
        ws.send(cmd);
      } else {
        alert("Please connect first");
      }
    }

    function setBaud() {
      var cmd = document.getElementById("ddBaudrate").value;
      console.log(cmd);

      sendCommand("serial(0," + cmd + ")\n");
    }

    function getStatus() {
      sendCommand("status");
    }

    function connectToHost() {
      var ip = document.getElementById("txtHostIP").value;

      launchWS(ip);
    }

    function setDate() {
      let date = new Date();
      console.log(date);
      console.log(date.getTimezoneOffset() / 60);
      date.setHours(date.getHours() - date.getTimezoneOffset() / 60);
      console.log(date);

      var dateStr = date.toJSON().slice(0, 19);

      console.log(dateStr); // 2022-06-17T11:06:50.369Z

      sendCommand("set_clock(" + dateStr + ")");
    }

    function launchWS(ip) {
      if ("WebSocket" in window) {
        if (ws != null) console.log(ws.readyState);

        if (ws != null && ws.readyState === WebSocket.CLOSING) {
        }
        if (ws != null && ws.readyState === WebSocket.OPEN) {
          ws.onclose = null;
          ws.close();
          document.getElementById("btnConnect").style.background = "#FF0000";
        } else {
          ws = new WebSocket("ws://" + ip + "/ws");

          ws.onopen = function () {
            document.getElementById("btnConnect").style.background = "#00FF00";
          };

          var received_msg;

          ws.onmessage = async function (evt) {
            if (evt.data instanceof Blob) {
              reader = new FileReader();

              reader.onload = () => {
                // console.log("Result: " + reader.result);
                received_msg = reader.result;

                if (received_msg != null) {
                  // Check the data source
                  if (received_msg.startsWith("RX1,")) {
                    $("#textarea1").val(
                      $("#textarea1").val() + received_msg.substring(4)
                    );

                    if (_autoScroll) {
                      document.getElementById("textarea1").scrollTop =
                        document.getElementById("textarea1").scrollHeight;
                    }
                  } else if (received_msg.startsWith("RX2,")) {
                    $("#textarea2").val(
                      $("#textarea2").val() + received_msg.substring(4)
                    );
                    if (_autoScroll) {
                      document.getElementById("textarea2").scrollTop =
                        document.getElementById("textarea2").scrollHeight;
                    }
                  } else if (received_msg.startsWith("version(")) {
                    // Split the message
                    vv = received_msg.substring(8, received_msg.length - 1);

                    received_msg.split(";").forEach(function (v) {
                      if (v.startsWith("version")) {
                        ind = v.indexOf(")");
                        _version = v.substring(8, ind);
                        console.log(_version);
                        document.getElementById("title").innerText =
                          "Wibee Uart WiFi Bridge v" + _version;
                      } else if (v.startsWith("id(")) {
                        ind = v.indexOf(")");
                        _socketId = v.substring(3, ind);
                        console.log(_socketId);
                        document.getElementById("title").innerText =
                          "Wibee Uart WiFi Bridge v" +
                          _version +
                          " id: " +
                          _socketId;
                      }
                    });
                  } else {
                    $("#textarea1").val(
                      $("#textarea1").val() + ("ESP:" + received_msg)
                    );
                  }
                }
              };

              await reader.readAsText(evt.data);
            } else {
              console.log("Result: " + evt.data);
            }
          };

          ws.onclose = function () {
            // websocket is closed.
            // alert("Connection is closed...");
            console.log("Websocket Dropped");
            document.getElementById("btnConnect").style.background = "#FF0000";

            ws = null;
          };
        }
      } else {
        alert("WebSocket NOT supported by your Browser!");
      }
    }

    function clear_console() {
      document.getElementById("textarea1").value = "";
      document.getElementById("textarea2").value = "";
    }

    function doc_keyUp(e) {
      // this would test for whichever key is 40 (down arrow) and the ctrl key at the same time
      if (e.key == "C" && e.shiftKey) {
        clear_console();
      }
    }

    function enLogClicked() {
      // Get the checkbox
      var checkBox = document.getElementById("cbEnLog");

      // If the checkbox is checked, display the output text
      if (checkBox.checked == true) {
        sendCommand("enlog(1)");
      } else {
        sendCommand("enlog(0)");
      }
    }

    function enAutoScrollClicked() {
      // Get the checkbox
      var checkBox = document.getElementById("cbEnAutoScroll");

      // If the checkbox is checked, display the output text
      _autoScroll = checkBox.checked;

      if (_autoScroll) {
        document.getElementById("textarea1").scrollTop =
          document.getElementById("textarea1").scrollHeight;
        document.getElementById("textarea2").scrollTop =
          document.getElementById("textarea2").scrollHeight;
      }
    }

    function onDelLog() {
      // Get the checkbox
      if (confirm("Are you sure you want to delete the Log file?") == true) {
        sendCommand("dellog()");
      } else {
      }
    }
    document.addEventListener("keyup", doc_keyUp, false);
  </script>
</html>

    )rawliteral";

#endif