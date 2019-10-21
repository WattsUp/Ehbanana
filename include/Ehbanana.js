var ehbanana = {
  webSocket: null,
  webSocketAddress:
      "ws://" + window.location.hostname + ":" + window.location.port,

  /**
   * On receiving a message from the WebSocket
   * @param {WebSocket event} event
   */
  webSocketMessage: function(event) {
    var jsonEvent = JSON.parse(event.data);
    if (jsonEvent.href != window.location.pathname && jsonEvent.href != "")
      return;
    for (var id in jsonEvent.elements) {
      var obj = document.getElementById(id);
      if (!obj) {
        console.log("Ehbanana Message #" + id + " not found");
        continue;
      }

      for (var key in jsonEvent.elements[id]) {
        obj[key] = jsonEvent.elements[id][key];
      }
      if (obj.classList.contains("eb-metric-prefix"))
        ehbanana.numToMetricPrefix(obj);
      if (obj.hasAttribute("ebcallback")) {
        var func = window[obj.getAttribute("ebcallback")];
        if (typeof func == "function")
          func(obj);
      }
    }
    if (document.body.hasAttribute("ebmsg-callback")) {
      var func = window[document.body.getAttribute("ebmsg-callback")];
      if (typeof func == "function")
        func();
    }
  },

  /**
   * Listen to an input event and send the results to the WebSocket
   * @param {InputEvent} event
   */
  listenerInput: function(event) {
    var jsonEvent = {
      href: window.location.pathname,
      id: event.target.id,
      value: "" + event.target.value
    };

    // populate data with appropriate information
    if (event.target.type == "checkbox")
      jsonEvent.value = "" + event.target.checked;

    if (event.target.type == "file") {
      jsonEvent.fileSize = event.target.files[0].size;
    }

    ehbanana.webSocket.send(JSON.stringify(jsonEvent));

    if (event.target.type == "file") {
      ehbanana.webSocket.send(event.target.files[0]); // Sent as binary
    }
  },

  /**
   * Listen to an enter key event and send the results to the WebSocket
   * @param {KeyupEvent} event
   */
  listenerEnter: function(event) {
    if (event.keyCode != 13)
      return;
    event.target.blur();
    ehbanana.listenerInput(event);
  },

  /**
   * Add a listener to each element with "eb-*" class
   */
  attachListeners: function() {
    if (ehbanana.webSocket == null ||
        ehbanana.webSocket.readyState != ehbanana.webSocket.OPEN) {
      setTimeout(ehbanana.attachListeners, 20);
      return;
    }
    var elements = document.getElementsByClassName("eb-input");
    for (var i = 0; i < elements.length; i++) {
      if (elements[i].getAttribute("id") == null) {
        if (elements[i].getAttribute("name") == null) {
          console.log("No name nor id for eb-input:", elements[i]);
          continue;
        }
        elements[i].setAttribute("id", elements[i].name);
      }
      if (elements[i].getAttribute("type") == "button")
        elements[i].addEventListener("click", ehbanana.listenerInput);
      else
        elements[i].addEventListener("input", ehbanana.listenerInput);
    }
    elements = document.getElementsByClassName("eb-onload");
    for (var i = 0; i < elements.length; i++) {
      if (elements[i].getAttribute("id") == null) {
        if (elements[i].getAttribute("name") == null) {
          console.log("No name nor id for eb-onload:", elements[i]);
          continue;
        }
        elements[i].setAttribute("id", elements[i].name);
      }
      var jsonEvent = {
        href: window.location.pathname,
        id: elements[i].id,
        value: "on-load-update"
      };
      ehbanana.webSocket.send(JSON.stringify(jsonEvent));
    }
    elements = document.getElementsByClassName("eb-onenter");
    for (var i = 0; i < elements.length; i++) {
      if (elements[i].getAttribute("id") == null) {
        if (elements[i].getAttribute("name") == null) {
          console.log("No name nor id for eb-onload:", elements[i]);
          continue;
        }
        elements[i].setAttribute("id", elements[i].name);
      }
      elements[i].addEventListener("keyup", ehbanana.listenerEnter);
    }
  },

  /**
   * Send a request for the websocket port
   * Open a websocket to that port
   */
  startWebsocket: function() {
    ehbanana.webSocket           = new WebSocket(ehbanana.webSocketAddress);
    ehbanana.webSocket.onmessage = ehbanana.webSocketMessage;
    ehbanana.webSocket.onclose   = function(event) {
      var webSocketStatus = document.getElementById("websocket-status");
      if (webSocketStatus)
        webSocketStatus.innerHTML =
            "Closed connection to " + ehbanana.webSocketAddress;
      if (document.body) {
        document.body.classList.remove("ehbanana-error");
        document.body.classList.remove("ehbanana-opened");
        document.body.classList.add("ehbanana-closed");
      }
    };
    ehbanana.webSocket.onopen = function(event) {
      var webSocketStatus = document.getElementById("websocket-status");
      if (webSocketStatus)
        webSocketStatus.innerHTML =
            "Opened connection to " + ehbanana.webSocketAddress;
      if (document.body) {
        document.body.classList.remove("ehbanana-error");
        document.body.classList.add("ehbanana-opened");
        document.body.classList.remove("ehbanana-closed");
      }
    };
    ehbanana.webSocket.onerror = function(event) {
      var webSocketStatus = document.getElementById("websocket-status");
      if (webSocketStatus)
        webSocketStatus.innerHTML =
            "Could not connect to " + ehbanana.webSocketAddress;
      if (document.body) {
        document.body.classList.add("ehbanana-error");
        document.body.classList.remove("ehbanana-opened");
        document.body.classList.remove("ehbanana-closed");
      }
    };
  },

  /**
   * Set the innerHTML and value of the element to its num with proper metric
   * prefix and its unit and number of significant figures
   *
   * attributes: unit, sigfig
   * @param {DOMElement} element to process
   */
  numToMetricPrefix: function(element) {
    if (element == document.activeElement)
      return; // Don't change the focused element

    var num = element["num"];
    if (String(num).length == 0) {
      element.innerHTML = "";
      element.value     = "";
      return;
    }
    if (num == "infinity") {
      element.innerHTML = "infinity";
      element.value     = "infinity";
      return;
    }
    var exp    = Math.floor(Math.log10(Math.abs(num)) / 3);
    exp        = Math.min(4, Math.max(-4, exp));
    num        = (num / Math.pow(1000, exp));
    num        = num.toPrecision(element.getAttribute("sigfig"));
    var prefix = ["p", "n", "µ", "m", "", "k", "M", "G", "T"][exp + 4];
    if (num == 0) {
      prefix = "";
    }
    var unit = element["unit"];
    if (!unit)
      unit = element.getAttribute("unit");
    element.innerHTML = num + " " + prefix + unit;
    element.value     = num + " " + prefix + unit;
  }
};

ehbanana.startWebsocket();
document.addEventListener("DOMContentLoaded", ehbanana.attachListeners);