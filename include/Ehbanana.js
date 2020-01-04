let ehbanana = {
  webSocket: null,
  webSocketAddress:
      "ws://" + window.location.hostname + ":" + window.location.port,

  /**
   * On receiving a message from the WebSocket
   * @param {WebSocket event} event
   */
  webSocketMessage: function(event) {
    let jsonEvent;
    try {
      jsonEvent = JSON.parse(event.data);
    } catch (error) {
      console.log("Failed t parse WebSocket data {}\n{}", error, event);
    }
    if (jsonEvent.href != window.location.pathname && jsonEvent.href != "")
      return;
    for (let id of jsonEvent.elements) {
      let obj = document.getElementById(id);
      if (!obj) {
        console.log("Ehbanana Message #" + id + " not found");
        continue;
      }

      for (let key of jsonEvent.elements[id]) {
        obj[key] = jsonEvent.elements[id][key];
      }
      if (obj.classList.contains("eb-metric-prefix"))
        ehbanana.numToMetricPrefix(obj);
      if (obj.hasAttribute("ebcallback")) {
        let func = window[obj.getAttribute("ebcallback")];
        if (typeof func == "function")
          func(obj);
      }
    }
    if (document.body.hasAttribute("ebmsg-callback")) {
      let func = window[document.body.getAttribute("ebmsg-callback")];
      if (typeof func == "function")
        func();
    }
  },

  /**
   * Listen to an input event and send the results to the WebSocket
   * @param {InputEvent} event
   */
  listenerInput: function(event) {
    if (event.target.type == "file") {
      for (let file of event.target.files) {
        let jsonEvent = {
          href: window.location.pathname,
          id: event.target.id,
          value: file.name,
          fileSize: file.size
        };
        ehbanana.webSocket.send(JSON.stringify(jsonEvent));

        ehbanana.webSocket.send(file);
      }
      return;
    }
    let jsonEvent = {
      href: window.location.pathname,
      id: event.target.id,
      value: "" + event.target.value
    };

    // populate data with appropriate information
    if (event.target.type == "checkbox")
      jsonEvent.value = "" + event.target.checked;

    ehbanana.webSocket.send(JSON.stringify(jsonEvent));
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
    let elements = document.getElementsByClassName("eb-input");
    for (let element of elements) {
      if (element.getAttribute("id") == null) {
        if (element.getAttribute("name") == null) {
          console.log("No name nor id for eb-input:", element);
          continue;
        }
        element.setAttribute("id", element.name);
      }
      if (element.getAttribute("type") == "button")
        element.addEventListener("click", ehbanana.listenerInput);
      else
        element.addEventListener("input", ehbanana.listenerInput);
    }
    elements = document.getElementsByClassName("eb-onload");
    for (let element of elements) {
      if (element.getAttribute("id") == null) {
        if (element.getAttribute("name") == null) {
          console.log("No name nor id for eb-onload:", element);
          continue;
        }
        element.setAttribute("id", element.name);
      }
      let jsonEvent = {
        href: window.location.pathname,
        id: element.id,
        value: "on-load"
      };
      ehbanana.webSocket.send(JSON.stringify(jsonEvent));
    }
    elements = document.getElementsByClassName("eb-onenter");
    for (let element of elements) {
      if (element.getAttribute("id") == null) {
        if (element.getAttribute("name") == null) {
          console.log("No name nor id for eb-onload:", element);
          continue;
        }
        element.setAttribute("id", element.name);
      }
      element.addEventListener("keyup", ehbanana.listenerEnter);
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
      console.log("Close");
      let webSocketStatus = document.getElementById("websocket-status");
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
      console.log("Open");
      let webSocketStatus = document.getElementById("websocket-status");
      if (webSocketStatus)
        webSocketStatus.innerHTML =
            "Opened connection to " + ehbanana.webSocketAddress;
      if (document.body) {
        document.body.classList.remove("ehbanana-error");
        document.body.classList.add("ehbanana-opened");
        document.body.classList.remove("ehbanana-closed");
      }
      if (document.readyState == "loading") {
        document.addEventListener("DOMContentLoaded", ehbanana.attachListeners);
      } else {
        ehbanana.attachListeners();
      }
    };
    ehbanana.webSocket.onerror = function(event) {
      console.log("Error");
      let webSocketStatus = document.getElementById("websocket-status");
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
   * prefix, its unit and number of significant figures
   *
   * attributes: unit, sigfig
   * @param {DOMElement} element to process
   */
  numToMetricPrefix: function(element) {
    if (element == document.activeElement)
      return; // Don't change the focused element

    let num = element["num"];
    if (typeof (num) != "number") {
      element.innerHTML = num;
      element.value     = num;
      return;
    }
    let exp    = Math.floor(Math.log10(Math.abs(num)) / 3);
    exp        = Math.min(4, Math.max(-4, exp));
    num        = (num / Math.pow(1000, exp));
    num        = num.toPrecision(element.getAttribute("sigfig"));
    let prefix = ["p", "n", "µ", "m", "", "k", "M", "G", "T"][exp + 4];
    if (num == 0) {
      prefix = "";
    }
    let unit = element["unit"];
    if (!unit)
      unit = element.getAttribute("unit");
    element.innerHTML = num + " " + prefix + unit;
    element.value     = num + " " + prefix + unit;
  }
};

ehbanana.startWebsocket();