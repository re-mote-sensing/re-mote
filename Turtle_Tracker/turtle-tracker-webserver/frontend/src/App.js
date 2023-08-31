import React, { useEffect, useState } from "react";
import "./App.css";
import Header from "./components/Header/Header";
import History from "./components/History/History";
import Settings from "./components/Settings/Settings";
import { MapContainer, TileLayer, Marker, Popup, Tooltip, Polyline, LayersControl, LayerGroup, AttributionControl } from 'react-leaflet';
import * as L from "leaflet";
import './i18n';
import { useTranslation } from 'react-i18next';

function App() {
  // Server Address
  const server_host = 'https://turtletracker.cas.mcmaster.ca/events?stream=messages';
  // Coords data conversion
  const conversion_rate = 10e6;
  // Set the center and zoom level of the map
  const center = [43.053619, -80.102545];
  const zoom = 12;
  // PolyLines Color
  const lineOptions = { color: '#7e3e98' };
  
  // Panel state: open or close
  const [paneState1, setPaneState1] = useState({isPaneOpen: false});
  const [paneState2, setPaneState2] = useState({isPaneOpen: false});
  const [map, setMap] = useState(null)

  const { t } = useTranslation();
  const [log, setLog] = useState([]);
  const [polylines, setPolylines] = useState([]);

  // Marker Logo on the map, id = tracker id
  // TODO: Different Color with different id
  function getMarkerIcon(id) {
    return new L.Icon({
      iconUrl: "/turtle-marker.png",
      iconSize: [25, 25],
      popupAnchor: [0, -13],
      tooltipAnchor: [13, 0],
    });
  }

  // Get formatted time with timestamp
  function time(time){
    console.log(time);
    if (time === "1970-01-01 00:00:00 +0000 UTC")
      return "Unset";
    let t = Date.parse(time)
    if (isNaN(t))
      // Fix on Safari
      t = Date.parse(time.replace(" ","T").substr(0,19)+"Z")
    return new Date(t).toLocaleString();
  }

  useEffect (() => {

    // Server SSE Address
    const sse = new EventSource(server_host);
    //const sse = new EventSource('https://domain/events?stream=messages', { withCredentials: true });

    // Store coords for each tracker, used for PolyLines route
    var lines = {}

    function parseEventData(data) {

      // Parse date into human readable
      data.time = time(data.time)

      // Coords Data Conversion
      data.lat = data.lat/conversion_rate;
      data.lon = data.lon/conversion_rate;

      // Set marker and history data
      setLog(old => [data, ...old]);

      // Save coords for drawing trace route
      if (!(data.id in lines))
        lines[data.id] = []
      lines[data.id].push([data.lat, data.lon]);
      reGeneratePolyLines();
    }

    // Generate Route of Turtle
    function reGeneratePolyLines(){
      var multiPolyline = [];
      for (const [key] of Object.entries(lines))
        multiPolyline.push(lines[key])
      setPolylines(old => multiPolyline);
    }

    sse.onmessage = (event) => {
      parseEventData(JSON.parse(event.data))
    }

    sse.onerror = () => {
      console.log("SSE Error");
      sse.close();
    }

    return () => {
      sse.close();
    };
  }, []);

  // Renders the app
  return (
    <div className="App">
      <Header setPaneState1={setPaneState1} setPaneState2={setPaneState2} />

      {/* Main Map */}
      <MapContainer
        center={center}
        zoom={zoom}
        scrollWheelZoom={true}
        attributionControl={false}
        ref={setMap}
      >
        <TileLayer
          attribution='<a href="https://leafletjs.com/" target="_blank">Leaflet</a> | &copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors'
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
          maxNativeZoom={19}
          maxZoom={25}
        />
        <AttributionControl prefix={false} />

        <LayersControl position="topright">
          <LayersControl.Overlay checked name={t('turtle tracker')}>
            <LayerGroup>
              {
                log.map(function(data, i){
                  return (<Marker key={i} position={[data.lat,data.lon]} alt={data.id} icon={getMarkerIcon(data[2])}>
                    <Popup>
                      <b>{t('tracker')} {data.id}</b><br/>
                      {data.time}<br/>
                      {data.lat}, {data.lon}
                    </Popup>
                    <Tooltip>{t('tracker')} {data.id}</Tooltip>
                    </Marker>);
                })
              }
            </LayerGroup>
          </LayersControl.Overlay>

          <LayersControl.Overlay checked name={t('trace route')}>
            <LayerGroup>
              <Polyline pathOptions={lineOptions} positions={polylines} />
            </LayerGroup>
          </LayersControl.Overlay>
          
        </LayersControl>

      </MapContainer>

      {/* Tracker History */}
      <History log={log}
        paneState1={paneState1}
        setPaneState1={setPaneState1}
        map={map}
      />
      {/* Settings */}
      <Settings
        paneState2={paneState2}
        setPaneState2={setPaneState2}
      />
    </div>
  );
}

export default App;