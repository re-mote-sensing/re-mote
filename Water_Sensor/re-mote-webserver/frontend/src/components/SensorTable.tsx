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
import config from "../config/config"

const api_url = config.api;

export interface SensorTableProps {
    location: Location,
    sensorType: string
}

interface SensorTableState {
    error: null | any,
    isLoaded: boolean,
    chartData: { any },
    result?: {
        id: string,
        type: string,
        data: Array<[number, number]>
    }
}

class SensorTable extends React.PureComponent<SensorTableProps, SensorTableState> {
    constructor(props) {
        super(props);
        this.state = {
            error: null,
            isLoaded: false,
            chartData: { any },
        }
        this.updateData();
    }
    updateData() {
        const { location, sensorType } = this.props;
        fetch(api_url+`/sensor/data?id=${location.id}&type=${sensorType}`)
            .then(res => res.json())
            .then(
                result => this.setState({ result, isLoaded: true, /**/error: null }),
                error => this.setState({ error, isLoaded: true })
            );
            console.log(this.state.result);
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
        console.log(result)

        if (this.state.result.type === 'Temperature') {
            var yAxisLabel: string = '°C';
        }
        else if (this.state.result.type === 'Air_Temperature') {
            var yAxisLabel: string = '°C';
        }
        else if (this.state.result.type === 'Water_Temperature') {
            var yAxisLabel: string = '°C';
        }
        else if (this.state.result.type === 'Dissolved_Oxygen') {
            var yAxisLabel: string = 'mg/L';
        }
        else if (this.state.result.type === 'Turbidity') {
            var yAxisLabel: string = 'Units - NA';
        }
        else if (this.state.result.type === 'Conductivity') {
            var yAxisLabel: string = 'µS/cm';
        }
        else {
            var yAxisLabel: string = 'Units - NA';
        }

        if (result.data && result.data.length > 0) {
            return (
                <div>
                    <br />
                    <div>
                        <Translation>
                            {
                                t => <h5>{t('sensor-table.title', { type: this.state.result.type })}</h5>
                            }
                        </Translation>
                        <Translation>
                            {
                                t => <h6>{t('sensor-table.location', { name: this.props.location.name })}</h6>
                            }
                        </Translation>
                        <Chart chartData={this.state.result.data} Parameter={this.state.result.type} />
                    </div>

                    <br />

                      <Accordion>
                        <AccordionSummary
                          expandIcon={<ExpandMoreIcon />}
                          aria-controls="panel1a-content"
                          id="panel1a-header"
                        >
                          <Typography>Full Data</Typography>
                        </AccordionSummary>
                        <AccordionDetails>
                          <TableContainer component={Paper}>
                            <Table size="small" aria-label="a dense table">
                                <TableHead>
                                    <TableRow>
                                        <TableCell>ID</TableCell>
                                        <Translation>
                                            {
                                                t => <TableCell>{t('sensor-table.time')}</TableCell>
                                            }
                                        </Translation>
                                        <Translation>
                                            {
                                                t => <TableCell>{t('sensor-table.value', { label: yAxisLabel })}</TableCell>
                                            }
                                        </Translation>
                                    </TableRow>
                                </TableHead>
                                <TableBody>
                                    {result.data.map((item, index) => (
                                        <TableRow>
                                            <TableCell>{index}</TableCell>
                                            <TableCell>{(new Date(item[0] / 1000000)).toLocaleString()}</TableCell>
                                            <TableCell>{Number(item[1]).toFixed(2)}</TableCell>
                                        </TableRow>
                                    ))}
                                </TableBody>
                            </Table>
                        </TableContainer>
                        </AccordionDetails>
                      </Accordion>

                      <br /><br /><br /><br />
                    
                </div>

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

export default SensorTable;
