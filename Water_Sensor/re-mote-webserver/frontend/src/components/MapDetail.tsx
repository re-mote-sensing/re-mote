import * as React from 'react';
import {Map, Marker, Popup, TileLayer, Polyline, CircleMarker, Tooltip} from 'react-leaflet';
import {Location} from '~/components/Map';
import { ScaleControl } from 'react-leaflet' 

interface MapDetailProps {
    locations: Location[][],
    selectedLocation: Location,
    path: number[][][],
    changeLocation: (location: Location) => void,
    changeType: (type: string) => void
}

class MapDetail extends React.PureComponent<MapDetailProps, null> {
    constructor(props) {
        super(props);
    }

    handleClickAvailable = event => {
        var coords = [event.latlng.lat, event.latlng.lng];
        const {locations} = this.props;
        for (var i = 0; i < locations[0].length; i++) {
            if (locations[0][i].coordinates[0] == coords[0] && locations[0][i].coordinates[1] == coords[1]) {
                this.props.changeLocation(locations[0][i]);
                break;
            }
        }
    };

    handleClickUnavailable = event => {
        var coords = [event.latlng.lat, event.latlng.lng];
        const {locations} = this.props;
        for (var i = 0; i < locations[1].length; i++) {
            if (locations[1][i].coordinates[0] == coords[0] && locations[1][i].coordinates[1] == coords[1]) {
                this.props.changeType("");
                this.props.changeLocation(locations[1][i]);
                break;
            }
        }
    };

    renderPath() {
        const {path} = this.props;
        if (path == null) {
            return null;
        }
        return (
            path.map(locs => {
                    return (
                        <>
                            <Polyline weight={5} positions={locs} dashArray={"20"} />
                            <CircleMarker radius={6} fillOpacity={1} center={locs[0]} />
                        </>
                    )
                })
        );
    }
    
    render(): React.ReactNode {
        const {locations, selectedLocation} = this.props;
        return (
            <Map center={[43.268383, -79.920265]} zoom={12}>
                <TileLayer
                    url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
                    attribution="&copy; <a href=&quot;http://osm.org/copyright&quot;>OpenStreetMap</a> contributors"
                />
                {locations[0].map(loc => {
                    if (loc == null) return null;
                    // Ignore GPS disabled devices
                    if (loc.coordinates[0] == 43 && loc.coordinates[1] == -79) return null;
                    return (
                        <Marker key={loc.id} position={loc.coordinates} title={loc.name} onClick={this.handleClickAvailable}>
                            <Popup>
                                #{loc.id} {loc.name}<br/>
                                {loc.coordinates}
                            </Popup>
                            <Tooltip>{loc.name}</Tooltip>
                        </Marker>
                    );
                })}
                {locations[1].map(loc => {
                    if (loc == null) return null;
                    // Ignore GPS disabled devices
                    if (loc.coordinates[0] == 43 && loc.coordinates[1] == -79) return null;
                    return (
                        <Marker key={loc.id} position={loc.coordinates} title={loc.name} onClick={this.handleClickUnavailable} /*icon={greyIcon}*/opacity={0.5}>
                            <Popup>
                                #{loc.id} {loc.name}<br/>
                                {loc.coordinates}
                            </Popup>
                            <Tooltip>{loc.name}</Tooltip>
                        </Marker>
                    );
                })}
                {this.renderPath()}
                <ScaleControl position={"bottomleft"} />
            </Map>
        );
    }
}

export default MapDetail;
