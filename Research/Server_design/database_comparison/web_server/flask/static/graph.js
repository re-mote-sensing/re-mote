var updateTaskID //when setInterval is called the need to make end the old session before starting a new one
function drawGraph(dbname, sensorType) {
    //data is obtained from the proxy server which retrives it from influx

    // Find the portion of the current location that corresponds to the base url.
    // This is the portion before the `/static/graph.html` which we know to be the
    // local path to the page we are viewing.
    baseUrl = window.location.href.match(/^(.*)\/static\/graph.html/)[1]

    address = baseUrl + '/getData/query?'

    // query = encodeURI('db='+dbname+'&q=select Value from ' + sensorType + ' limit 20&epoch=ns') //for running locally(debug)
    query = encodeURI('db=' + dbname + '&q=select value from ' + sensorType + '&epoch=ns') //for running on pi

    var lastTime;

    function extractDataPointFromResp(rawDataResp) {
        var result = rawDataResp.results[0];
        var series = result.series != null ? result.series : [];
        var values = series.length > 0 ? series[0].values : [];

        //formats the data to x,y value pairs
        return values.map(function (datapoint) {
            return [
                Math.ceil(datapoint[0] / 1000000), //convert ns to ms precision for javascript
                datapoint[1]
            ];
        });
    }

    //get all datapoints inside the database
    url = address + query;
    $.getJSON(url, function (rawDataResp) {
        var data = extractDataPointFromResp(rawDataResp);
        var lastTime = data.length > 0 ? data[data.length - 1][0] * 1000000 : null;

        if (updateTaskID != null) {
            clearInterval(updateTaskID);
        }

        //for units showing on graph
        switch (sensorType) {
            case 'turbidity':
                units = 'NTU';
                break;
            case 'DO':
                units = 'mg/L';
                break;
            case 'EC':
                units = 'microSiemems/cm';
                break;
            case 'temperature':
                units = 'Degree Celcius';
                break;
        }

        function capitalizeFirstLetter(string) {
            return string.charAt(0).toUpperCase() + string.slice(1);
        }

        // Create the chart
        Highcharts.stockChart('container', {
            chart: {
                events: {
                    load: function () {
                        // set up the updating of the chart for a new datapoint checks database every second
                        var series = this.series[0];
                        updateTaskID = setInterval(function () {
                            //getting the next value that was placed in database since last pull
                            var filter = lastTime != null ? ' where time>' + lastTime : '';
                            var query = encodeURI('db=' + dbname + '&q=select value from ' + sensorType + filter + '&epoch=ns');

                            url = address + query;
                            $.getJSON(url, function (rawDataResp) {
                                var data = extractDataPointFromResp(rawDataResp);
                                data.forEach(function (datapoint) {
                                    //add points to the graph
                                    series.addPoint(datapoint, true, false);
                                    lastTime = Math.max(datapoint[0], lastTime);
                                });
                            });
                        }, 10000); // checks every 10 seconds
                    }
                }
            },

            rangeSelector: {
                inputEnabled: false,
                selected: 0
            },

            title: {
                text: capitalizeFirstLetter(sensorType) + ' Sensor Readings'
            },

            xAxis: {
                type: 'datetime'
            },
            yAxis: {
                title: {
                    text: capitalizeFirstLetter(sensorType) + ' Level: ' + units
                }
            },

            series: [{
                name: capitalizeFirstLetter(sensorType) + ' Level',
                data: data,
                tooltip: {
                    valueDecimals: 2
                }
            }]
        });
        
        showPage();
    });
}
