var mymap

function drawMap(){
    mymap = L.map('mapid').setView([43.2568, -79.9007], 20);

    //using the openstreetmap tile layer (FREE)
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
      attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
  }).addTo(mymap);

  // if (checkedFunction('loc1')){
  // // var marker1 = L.marker([43.2609, -79.9192]).addTo(mymap).bindPopup("<b>Sensor 1</b><br>See data")
  // var marker1 = L.marker([43.2609, -79.9192]).addTo(mymap).bindPopup('<div><b>Sensor 2</b><br><a href="">AmmoniaOne Data</a><br><a href="#">AmmoniaTwo Data</a></div>')
  //
  // }

  if (checkedFunction('loc2')){
    var marker2 = L.marker([43.2568, -79.9007]).addTo(mymap)

    if(checkedFunction('turbidity')){
      var link1=$('<a href="#container" >Turbidity</a>').click(function (){
        drawGraph("sensors","turbidity")
      })
    }

    if(checkedFunction('DO')){
      var link2=$('<a href="#container">Dissolved Oxygen</a>').click(function(){
        drawGraph("sensors","DO")
      })
    }

    if(checkedFunction('EC')){
      var link3=$('<a href="#container" >Electrical Conductivity</a>').click(function (){
        drawGraph("sensors","EC")
      })
    }

    if(checkedFunction('temperature')){
      var link4=$('<a href="#container" >Temperature</a>').click(function (){
        drawGraph("sensors","temperature")
      })
    }

    marker2.bindPopup($('<div><b>McMaster Innovation Park</b><br></div>').append(link1,$('<br>'),link2,$('<br>'),link3,$('<br>'),link4)[0])
  }
}

function refreshMap(){
  mymap.off()
  mymap.remove()
  drawMap()
}
