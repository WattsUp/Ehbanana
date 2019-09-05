var config = {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      label: "Random number",
      backgroundColor: "rgb(211, 78, 36)",
      borderColor: "rgb(211, 78, 36)",
      data: [],
      fill: false,
    }]
  },
  options: {
    responsive: true,
    title: {display: true, text: "Chart.js Line Chart Streaming"},
    tooltips: {mode: "x"},
    hover: {mode: "x"},
    scales: {
      xAxes:
          [{display: true, scaleLabel: {display: true, labelString: "Time"}}],
      yAxes:
          [{display: true, scaleLabel: {display: true, labelString: "Value"}}]
    }
  }
};

var chart;
var index = 0;

function setupChart() {
  var context = document.getElementById("stream-out-chart").getContext("2d");
  chart       = new Chart(context, config);
}

function updateChart(element) {
  config.data.datasets[0].data.push(element.innerHTML);
  if (config.data.labels.length < 20) {
    config.data.labels.unshift(index);
    index--;
  }
  if (config.data.datasets[0].data.length > 20)
    config.data.datasets[0].data.shift();
  chart.update();
}

window.addEventListener("load", setupChart);