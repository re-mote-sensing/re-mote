import React, { Component } from "react";
import "./Settings.scss";
import SlidingPane from "react-sliding-pane";
import "react-sliding-pane/dist/react-sliding-pane.css";
import { withTranslation } from 'react-i18next';
import Select from 'react-select';

// Language Options in settings
const options = [
  { value: 'en', label: 'English' },
  { value: 'fr', label: 'français' },
  // { value: 'de', label: 'deutsch' },
  // { value: 'zh', label: '简体中文' },
];
var selectedOption = null

class Settings extends Component {
    constructor() {
        super();
        this.state = {bleNoti: ""};
    }
    handleChange = selectedOption => {
        // Change Language
        const { i18n } = this.props;
        i18n.changeLanguage(selectedOption['value']);
    };
    connectBLE = () => {
        const { t } = this.props;
        if (!navigator.bluetooth) {
            this.setState({bleNoti: t('browser not supported')});
            return;
        }
        let options = {};
        options.acceptAllDevices = true;
        navigator.bluetooth.requestDevice(options)
        .then(device => {
            this.setState({bleNoti: ['Name: '+device.name, <br />, 'ID: '+device.id, <br />, 'Connected: '+device.gatt.connected]});
            console.log(device);
        })
        .catch(error => {
            this.setState({bleNoti: error.toString()});
            console.log(error);
        });
    };
    render() {
        const { t } = this.props;
        return (
            <>
                {/* Settings */}
                <SlidingPane
                    className="right-panel-settings"
                    overlayClassName="right-panel-settings-overlay"
                    isOpen={this.props.paneState2.isPaneOpen}
                    title={t('settings')}
                    subtitle=""
                    onRequestClose={() => {
                        this.props.setPaneState2({ isPaneOpen: false });
                        this.setState({bleNoti: ""}); // Clear BLE Msg
                    }}
                >
                    {/*<center>
                        <h1 className="title">re-mote</h1>
                        <p className="subtitle">{t('Department of Computing and Software')}</p>
                        <p className="subtitle">{t('McMaster University')}</p>
                    </center>
                    <br/><hr/>*/}

                    <center>
                        <h1 className="title">Six Nations</h1>
                        <p className="subtitle">of The Grand River</p>
                        <p className="subtitle">https://www.sixnations.ca/</p>
                    </center>
                    <br/><hr/>

                    {/* Language Select */}
                    <h4>Select Language{t('select language')}</h4>
                    <Select
                        value={selectedOption}
                        onChange={this.handleChange}
                        options={options}
                      />

                    {/*<br/><hr/>*/}

                    {/* Bluetooth Action */}
                    {/*<h4>{t('bluetooth')}</h4>

                    <button onClick={this.connectBLE}>
                        {t('bluetooth connect')}
                    </button>

                    {this.state.bleNoti==="" ? '' : <p className="noti">{this.state.bleNoti}</p>}*/}

                </SlidingPane>
            </>
        )
    }
}

export default withTranslation()(Settings);