var mymap
var locationJson = {
  "locations": [{
      "id": "12345",
      "name": "McMaster Innovation Park",
      "location": [43.2568, -79.9007],
      "tag": "not created",
    },
    {
      "id": "12346",
      "name": "firstLocation",
      "location": [43.25, -79.90],
      "tag": "not created",
    },
    {
      "id": "12347",
      "name": "secondLocation",
      "location": [43.257, -79.9],
      "tag": "not created",
    },
    {
      "id": "12348",
      "name": "thirdLocation",
      "location": [43.256, -79.9],
      "tag": "not created",
    }
  ]
};
//$.getJSON(url,data,success)

var database = $.get("http://localhost:8086/query?db=firstdb&epoch=ms&q=SELECT+*+FROM+%22sensorinfo%22").done(
  function (data) {
    return data
  });

function createList() {
  locationJson.locations.forEach(item => {
    if (item.tag === "not created") {
      let locationName = item.name;
      let appendLocation = "<tr id=" + item.id + "><td><div class='form-check'> <label class='form-check-label'> <input class='form-check-input' type='checkbox' id='" + locationName + "' value=''  checked><span class='form-check-sign'><span class='check'></span></span></label></div></td><td>" + locationName + "</td></tr>";
      $('.locationList').append(appendLocation);
      item.tag = "created";
    }
  });
}

function sensorInfo(sensortype, location) {
  let databaseJSON = database.responseJSON.results[0].series[0].values;
  let sensorText;
  databaseJSON.map((item) => {
    if (location === item[1] && sensortype === item[2]){
      sensorText = location + " " + sensortype + " is " + item[3];
    } 
  });
  // Get the snackbar DIV
  if(sensorText === undefined){
    sensorText = "info unavailable"
  }
  var x = $('#snackbar')[0];
  $('#snackbar').text(sensorText);
  // Add the "show" class to DIV
  x.className = "show";

  // After 10 seconds, remove the show class from DIV
  setTimeout(function () {
    x.className = x.className.replace("show", "");
  }, 10000);
};

function drawMap() {
  createList();
  mymap = L.map('mapid').setView([43.2568, -79.9007], 15);
  locationJson.locations.forEach(item => {
    if (checkedFunction(item.name)) {
      var marker2 = L.marker(item.location).addTo(mymap);
      
      if (checkedFunction('turbidity')) {
        var link1 = $('<a href="#container" >Turbidity</a>').click(function () {
          drawGraph("sensors", "turbidity");
          sensorInfo(this.innerText, item.name);
        })
      }

      if (checkedFunction('DO')) {
        var link2 = $('<a href="#container">DissolvedOxygen</a>').click(function () {
          drawGraph("sensors", "DO");
          sensorInfo(this.innerText, item.name);
        })
      }

      if (checkedFunction('EC')) {
        var link3 = $('<a href="#container" >ElectricalConductivity</a>').click(function () {
          drawGraph("sensors", "EC");
          sensorInfo(this.innerText, item.name);
        })
      }

      if (checkedFunction('temperature')) {
        var link4 = $('<a href="#container" >Temperature</a>').click(function () {
          drawGraph("sensors", "temperature");
          sensorInfo(this.innerText, item.name);
        })
      }

      marker2.bindPopup($('<div><b>'+item.name+'</b><br></div>').append(link1, $('<br>'), link2, $('<br>'), link3, $('<br>'), link4)[0])
    }
  });

  //using the openstreetmap tile layer (FREE)
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
  }).addTo(mymap);

}

function refreshMap() {
  mymap.off()
  mymap.remove()
  drawMap()
}