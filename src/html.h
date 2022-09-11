
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
      console.log(window.location.hostname);

      window.addEventListener("load", (event) => {
        console.log("page is fully loaded");

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
            ws.onmessage = async function (evt) {
              if (evt.data instanceof Blob) {
                reader = new FileReader();

                reader.onload = () => {
                  // console.log("Result: " + reader.result);
                  var received_msg = reader.result;

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
                      vv = received_msg.substring(8, received_msg.length - 1);
                      document.getElementById("btnConnect").value =
                        received_msg.substring(8, received_msg.length - 1);
                    } else {
                      console.log("Invalid Message " + received_msg);
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
    </script>
  </head>

  <body>
    <h1>Wibee Uart WiFi Bridge</h1>

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
      onClick='document.getElementById("textarea1").value =""; document.getElementById("textarea2").value =""'
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