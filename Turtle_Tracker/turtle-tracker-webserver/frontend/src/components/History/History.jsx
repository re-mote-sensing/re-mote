import React, { Component } from "react";
import "./History.scss";
import SlidingPane from "react-sliding-pane";
import "react-sliding-pane/dist/react-sliding-pane.css";
import { withTranslation } from 'react-i18next';

class History extends Component {
    render() {
        const { t } = this.props;
        const track_data = this.props.log.map((msg, index) => (
            <p key={index}>
                {/* Button that finding tracker on the map when map is ready */}
                {this.props.map ? 
                    <button className="tracker_button" onClick={() => {
                        this.props.map.setView([msg.lat,msg.lon], 16);
                        this.props.setPaneState1({ isPaneOpen: false });
                    }}>
                        {t('tracker')} #{msg.id}
                    </button>
                    : <b>{t('tracker')} #{msg.id}</b>
                }
                 <br className="onMobile" />
                [ {msg.time} ] <br className="onMobile" />
                [ {msg.lat===0?("Unset"):(msg.lat)}, {msg.lon===0?("Unset"):(msg.lon)} ] <br className="onMobile" />
                ( {msg.count}, {msg.battery}, {msg.temperature} )
            </p>
        ));
        return (
            <>
                {/* Tracker History */}
                <SlidingPane
                    className="right-panel-log"
                    overlayClassName="right-panel-log-overlay"
                    isOpen={this.props.paneState1.isPaneOpen}
                    title={t('tracker history')}
                    subtitle=""
                    onRequestClose={() => {
                        this.props.setPaneState1({ isPaneOpen: false });
                    }}
                >
                    <h4>{t('notice-in-history')}</h4>

                    <p>
                        <button className="tracker_button">
                                {t('tracker')} #id
                            </button>
                            
                         <br className="onMobile" />
                        [ Time ] <br className="onMobile" />
                        [ Latitude, Longitude ] <br className="onMobile" /> ( Serial #, Battery, Temp )
                    </p><br/>

                    {track_data}
                </SlidingPane>
            </>
        )
    }
}

export default withTranslation()(History);