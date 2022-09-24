
#ifndef __HTTML___
#define __HTTML___

#include "pgmspace.h"

const char index_html[] PROGMEM = R"rawliteral(
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

    <script type="text/javascript">
      var _version;
      var _socketId;

      console.log(window.location.hostname);

      window.addEventListener("load", (event) => {
        console.log("page is fully loaded");

        if (window.location.hostname.startsWith("127.0.0.1"))
          document.getElementById("txtHostIP").value = "192.168.1.30";
        else
          document.getElementById("txtHostIP").value = window.location.hostname;
      });

      var ws = null;

      function sendMessage() {
        var cmd = document.getElementById("txtCmd").value;

        ws.send(cmd + "\n");
      }

      function connectToHost() {
        var ip = document.getElementById("txtHostIP").value;

        launchWS(ip);
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
              document.getElementById("btnConnect").style.background =
                "#00FF00";
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
                    } else if (received_msg.startsWith("RX2,")) {
                      $("#textarea2").val(
                        $("#textarea2").val() + received_msg.substring(4)
                      );
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
                        $("#textarea1").val() +
                          ("ESP:" + received_msg)
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
              document.getElementById("btnConnect").style.background =
                "#FF0000";

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

      document.addEventListener("keyup", doc_keyUp, false);
    </script>
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

    <div class="row">
      <div class="column">
        <label for="textarea1">UART1</label>
        <br />
        <textarea id="textarea1"></textarea>
      </div>
      <div class="column">
        <label for="textarea1">UART2</label>
        <br />
        <textarea id="textarea2"></textarea>
      </div>
    </div>

    <input
      type="button"
      id="btnClear"
      onClick="clear_console()"
      value="Clear"
    />

    <br />
    <br />

    <input placeholder="Command" id="txtCmd" type="text" class="validate" />

    <input type="button" id="btnSend" onClick="sendMessage()" value="Send" />
  </body>
</html>


    )rawliteral";

#endif