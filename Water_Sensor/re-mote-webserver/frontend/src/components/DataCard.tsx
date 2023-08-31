import * as React from 'react';
import { Location } from '~/components/Map';
import Chart from '~/components/Chart';
import { any } from 'prop-types';
import { Translation } from 'react-i18next';
import LinearProgress from '@material-ui/core/LinearProgress';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableContainer from '@material-ui/core/TableContainer';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import Paper from '@material-ui/core/Paper';
import Alert from '@material-ui/lab/Alert';
import Accordion from '@material-ui/core/Accordion';
import AccordionSummary from '@material-ui/core/AccordionSummary';
import AccordionDetails from '@material-ui/core/AccordionDetails';
import Typography from '@material-ui/core/Typography';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import Card from '@material-ui/core/Card';
import CardContent from '@material-ui/core/CardContent';
import config from "../config/config"

const api_url = config.api;

export interface DataCardProps {
    location: Location,
    sensorType: string
}

interface DataCardState {
    error: null | any,
    isLoaded: boolean,
    chartData: { any },
    result?: {
        id: string,
        type: string,
        data: Array<[number, number]>
    }
}

class DataCard extends React.PureComponent<DataCardProps, DataCardState> {
    constructor(props) {
        super(props);
        this.state = {
            error: null,
            isLoaded: false,
            chartData: { any },
        }
        this.updateData();
        this.autoUpdateData();
    }
    updateData() {
        const { location, sensorType } = this.props;
        fetch(api_url+`/sensor/data?id=${location.id}&type=${sensorType}`)
            .then(res => res.json())
            .then(
                result => this.setState({ result, isLoaded: true, /**/error: null }),
                error => this.setState({ error, isLoaded: true })
            );
    }

    autoUpdateData() {
        var that = this;
        setTimeout(() => {
            console.log("Updating...");
            that.updateData();
            that.autoUpdateData();
        }, 5000);
    }


    componentDidUpdate(prevProps) {
        if (this.props != prevProps) {
            this.updateData();
        }
    }
    /*
    getChartData() {
        this.setState({
            chartData: 
        })
    }

    componentWillMount() {
        this.getChartData();
    }*/

    render() {
        const { isLoaded, error, result } = this.state;
        if (!isLoaded) return <LinearProgress color="secondary" />;
        if (error) return <Alert severity="error">Error {error}</Alert>;

        if (this.state.result.type === 'Temperature') {
            var yAxisLabel: string = '°C';
            var color: string = '#2f22a3';
        }
        else if (this.state.result.type === 'Air_Temperature') {
            var yAxisLabel: string = '°C';
            var color: string = '#2f22a3';
        }
        else if (this.state.result.type === 'Water_Temperature') {
            var yAxisLabel: string = '°C';
            var color: string = '#2f22a3';
        }
        else if (this.state.result.type === 'Dissolved_Oxygen') {
            var yAxisLabel: string = 'mg/L';
            var color: string = '#a3a322';
        }
        else if (this.state.result.type === 'Turbidity') {
            var yAxisLabel: string = '';
            var color: string = '#000000';
        }
        else if (this.state.result.type === 'Conductivity') {
            var yAxisLabel: string = 'µS/cm';
            var color: string = '#22a33a';
        }
        else if (this.state.result.type === 'pH') {
            var yAxisLabel: string = '';
            var color: string = '#a32222';
        }
        else {
            var yAxisLabel: string = '';
        }

        if (result.data && result.data.length > 0) {
            return (
                <Card>
                  <CardContent>
                    <Typography variant="h5" component="h2" style={{color: color}}>
                      {this.state.result.type}
                    </Typography>
                    <Typography variant="h6" component="p">
                      {Number(result.data[result.data.length-1][1]).toFixed(2)} {yAxisLabel}
                    </Typography>
                    <Typography color="textSecondary"style={{fontSize: 10}}>
                      {(new Date(result.data[result.data.length-1][0] / 1000000)).toLocaleString()}
                    </Typography>
                  </CardContent>
                </Card>

            );
        } else if (!result.data || error) {
            return <Translation>
                {
                    t => <div>{t('sensor-table.no-data', { name: this.props.location.name })}</div>
                }
            </Translation>;
        }
    }
}

export default DataCard;
