import {withStyles} from '@material-ui/core/styles';
import * as React from 'react';
import Menu from '~/components/Menu';
import MapDetail from '~/components/MapDetail';
import SensorTable from '~/components/SensorTable';
import DataCard from '~/components/DataCard';
import styles from '~/style/sensor-select-style';
import { Translation } from 'react-i18next';
import Button from '@material-ui/core/Button';
import Snackbar, { SnackbarOrigin } from '@material-ui/core/Snackbar';
import Alert from '@material-ui/lab/Alert';
import config from "../config/config"

const api_url = config.api;

export interface Location {
    id: string,
    name: string,
    coordinates: [number, number]
}

interface MapState {
    selectedType: string,
    selectedLocation: Location,
    path: number[][][],
    locations: Location[][],
    allLocations: Location[],
    types: string[][],
    allTypes: string[],
    open: boolean,
    snackbarmsg: string
}

class RealTime extends React.PureComponent<any, MapState> {
    constructor(props) {
        super(props);
        this.state = {
            selectedType: '',
            selectedLocation: null,
            path: null,
            locations: [[], []],
            allLocations: [],
            types: [[], []],
            allTypes: [],
            open: false,
            snackbarmsg: ""
        };
        this.changeLocation = this.changeLocation.bind(this);
        this.changeType = this.changeType.bind(this);
        this.updateLocations();
        this.updateTypes();
        this.getAllLocations();
        this.getAllTypes();
    }
    
    changeLocation(location) {
        this.setState({selectedLocation: location});
    }
    
    changeType(type) {
        this.setState({selectedType: type});
    }

    componentDidUpdate(prevProps, prevState) {
        const {locations, selectedType, selectedLocation} = this.state;
        if (selectedType != prevState.selectedType) {
            this.updateLocations();
        } 
        if (selectedLocation != prevState.selectedLocation) {
            var highestTimeoutId = setTimeout(";");
            for (var i = 0 ; i < highestTimeoutId ; i++) {
                clearTimeout(i); 
            }
            this.updateTypes();
            this.updatePath();
            // if (selectedLocation != null){
            //     this.setState({selectedType: ""});
            //     this.getTypes(selectedLocation).then((types) => {
            //         this.setState({selectedType: types[0]});
            //     })
            // }
        }
    }

    updateLocations() {
        const {selectedType, allLocations} = this.state;
        this.getLocs(selectedType).then(res => {
            if (res == undefined){
                this.handleSnackbar("There seems to be an error")
                return;
            }
            var result = this.parseLocResult(res);
            var ans = [result,[]];
            var j = 0;
            for (var i = 0; i < allLocations.length; i++) {
                if (j < result.length && allLocations[i].id == result[j].id) {
                    j++;
                } else {
                    ans[1].push(allLocations[i]);
                }
            }
            this.setState({locations: ans});
        });
    }

    updatePath() {
        const {selectedLocation} = this.state;
        if (selectedLocation == null) {
            this.setState({path: null});
            return;
        }
        var id = selectedLocation.id;
        fetch(api_url+`/sensor/info?id=${id}`)
            .then(res => res.json())
            .catch(err => this.handleSnackbar("There seems to be an error (/sensor/info/id): " + err ))
                .then(res => {
                    if (res == undefined){
                        this.handleSnackbar("There seems to be an error")
                        return;
                    }
                    if (res.locations.length <= 1) {
                        this.setState({path: null});
                        return;
                    }
                    var ans = [];
                    var coordsSplit1 = res.locations[0][1].split(",");
                    var lat1 = parseFloat(coordsSplit1[0]);
                    var lon1 = parseFloat(coordsSplit1[1]);
                    var coordsSplit2 = res.locations[1][1].split(",");
                    var lat2 = parseFloat(coordsSplit2[0]);
                    var lon2 = parseFloat(coordsSplit2[1]);
                    ans.push([[lat1, lon1], [lat2, lon2]]);
                    for (var i = 2; i < res.locations.length; i++) {
                        var currLine = [];
                        var coordsSplit = res.locations[i][1].split(",");
                        var lat = parseFloat(coordsSplit[0]);
                        var lon = parseFloat(coordsSplit[1]);
                        ans.push([ans[i-2][1], [lat, lon]]);
                    }
                    this.setState({path: ans});
                });
    }

    updateTypes() {
        const {selectedLocation, allTypes} = this.state;
        this.getTypes(selectedLocation).then(res => {
            var result = this.parseTypeResult(res);
            var ans = [result,[]];
            var j = 0;
            for (var i = 0; i < allTypes.length; i++) {
                if (j < result.length && allTypes[i] == result[j]) {
                    j++;
                } else {
                    ans[1].push(allTypes[i]);
                }
            }
            this.setState({types: ans});
        });
    }

    getAllLocations() {
        this.getLocs("").then(res => {
            if (res == undefined){
                this.handleSnackbar("There seems to be an error")
                return;
            }
            this.setState({allLocations: this.parseLocResult(res)});
            // Show by url param ?node=1
            const params = new URLSearchParams(location.search);
            const {allLocations} = this.state;
            if (params.get("node")){
                for (const location of allLocations){
                    if (location.id == params.get("node")){
                        this.setState({selectedLocation: location});
                    }
                }
            }
        });
    }

    getAllTypes() {
        this.getTypes(null).then(res => {
            this.setState({allTypes: this.parseTypeResult(res)});
        });
    }

    getLocs(selectedType) {
        return fetch(api_url+`/sensor/info?type=${selectedType}`)
            .then(res => res.json())
            .catch(err => this.handleSnackbar("There seems to be an error (/sensor/info/type): " + err ));
    }

    parseLocResult(Result) {
        var currArr = [];
        for (var i = 0; i < Result.locations.length; i++) {
            var coordsSplit = Result.locations[i][2].split(",");
            var lat = parseFloat(coordsSplit[0]);
            var lon = parseFloat(coordsSplit[1]);
            var currLocation = {
                id: Result.locations[i][0],
                name: Result.locations[i][1],
                coordinates: [lat, lon]
            };
            currArr.push(currLocation);
        }
        return currArr.sort(function (a, b) {
            return a.name.toUpperCase().localeCompare(b.name.toUpperCase());
        });
    }

    getTypes(selectedLoc) {
        var id;
        if (selectedLoc == null) {
            id = "";
        } else {
            id = selectedLoc.id;
        }
        return fetch(api_url+`/sensor/type?id=${id}`)
            .then(res => res.json())
            .catch(err => this.handleSnackbar("There seems to be an error (/sensor/type/id): " + err ));
    }

    parseTypeResult(Result) {
        var currArr = [];
        for (var i = 0; i < Result.types.length; i++) {
            currArr.push(Result.types[i]);
        }
        return currArr.sort(function (a, b) {
            return a.toUpperCase().localeCompare(b.toUpperCase());
        });
    }

    handleSnackbar = (msg) => {
        this.setState({ ...this.state, open: true, snackbarmsg:msg });
        console.log(msg);
    }

    handleClose = () => {
        this.setState({ ...this.state, open: false, snackbarmsg:"" });
    }

    render(): React.ReactNode {
        const {classes} = this.props;
        const {allTypes, selectedType, selectedLocation, locations, types, path, open, snackbarmsg} = this.state;

        return (
            <>
                <Snackbar
                    anchorOrigin={{ vertical: 'top', horizontal: 'right' }}
                    open={open}
                    onClose={this.handleClose}
                    message={snackbarmsg}
                    key="snackbar"
                  />
                <Menu 
                    classes={classes} 
                    selectedType={selectedType} 
                    selectedLocation={selectedLocation} 
                    locations={locations} 
                    types={types} 
                    changeLocation={this.changeLocation} 
                    changeType={this.changeType}
                />

                <div style={{ width: '100%', padding: '15px' }}>
                    <div>
                        <div className="row">
                            {selectedType !== '' && selectedLocation !== null
                            ? types[0].map((type) => 
                                <div className="col-md-4 col-xs-12" style={{ paddingBottom: '20px' }}>
                                    <DataCard sensorType={type} location={selectedLocation}/>
                                    <br/>
                                </div> )
                            : <></>}
                        </div>
                    </div>
                </div>
                
                {selectedType !== '' && selectedLocation !== null
                    ? <Alert severity="info">Data will refresh every 5 sec</Alert>
                    :   <Translation>
                        {
                            t => <><br /><Alert severity="info">{ t('map.select-type-location') }</Alert></>
                        }
                        </Translation> }
            </>
        );
    }
}

export default withStyles(styles as any, {withTheme: true})(RealTime);
