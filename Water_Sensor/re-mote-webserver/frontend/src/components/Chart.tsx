import * as React from 'react';
import { Line } from 'react-chartjs-3';
import { number } from 'prop-types';
import { ChartOptions } from 'chart.js'

interface ChartProps {
    displayTitle: boolean,
    displayLegend: boolean,
    legendPosition: string,
    chartData: Array<[number, number]>,
    Parameter: string,
}

interface ChartState {
    chartDataState: Array<[number, number]>
}

class Chart extends React.Component<ChartProps, ChartState>{
    constructor(props) {
        super(props);
        this.state = {
            chartDataState: props.chartData
        }
    }


    componentDidUpdate(prevProps) {
        if ((this.props != prevProps)) {
            this.setState({
                chartDataState: this.props.chartData
            })
        }
    }

    static defaultProps = {
        displayTitle: true,
        displayLegend: true,
        legendPosition: 'right',
        Parameter: 'Measured Parameter',
    }

    render() {
        //console.log(this.state.chartDataState)
        let EpochtimeData = this.state.chartDataState.map((time: number[]) => time[0])
        let HumantimeData = EpochtimeData.map(hTime => (new Date(hTime / 1000000)).toLocaleString())
        //console.log(timeData)
        let sensorValue = this.state.chartDataState.map((value: number[]) => value[1])
        //console.log(sensorValue)

        if (this.props.Parameter === 'Temperature') {
            var yAxisLabel: string = '°C';
        }
        else if (this.props.Parameter === 'Dissolved_Oxygen') {
            var yAxisLabel: string = 'mg/L';
        }
        else if (this.props.Parameter === 'Turbidity') {
            var yAxisLabel: string = 'Units';
        }
        else if (this.props.Parameter === 'Conductivity') {
            var yAxisLabel: string = 'µS/cm';
        }
        else {
            var yAxisLabel: string = 'Units';
        }

        const data = {
            labels: HumantimeData,
            datasets: [{
                label: this.props.Parameter,
                data: sensorValue,
                backgroundColor: 'rgba(255, 255, 255, 0)',
                borderColor: 'rgb(150, 93, 123)',
                borderWidth: 1.5,
                pointRadius: 1,
                pointHoverRadius: 5
            }]
        }
        let options: ChartOptions = {
            scales: {
                yAxes: [{
                    gridLines: {
                        drawBorder: false
                    },
                    scaleLabel: {
                        display: true,
                        labelString: yAxisLabel
                    }
                }],
				xAxes: [{
					gridLines: {
						display: false
                    }
				}]
            },
            title: {
                display: true,
                position: 'top',
                text: this.props.Parameter + ' vs Time'
            },
            legend: {
                display: false,
                position: 'bottom',
                labels: {
                    fontFamily: 'Comic Sans MS',
                    fontColor: 'rgb(133, 0, 69)',
                    boxWidth: 20,
                    boxFill: 'rgb(133, 0, 69)'
                }
            }
        };

        return (
            <div className="chart" style={{ backgroundColor: 'rgb(255,255,255)' }}>
                <Line data={data} options={options} />
            </div>           
        )
    }
}
export default Chart;