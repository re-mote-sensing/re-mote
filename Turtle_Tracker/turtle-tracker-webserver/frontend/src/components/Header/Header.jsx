import React, { Component } from "react";
import "./Header.scss";
import { withTranslation } from 'react-i18next';

class Header extends Component {
    render() {
        const { t } = this.props;
        return (
            <div className="header">
                <h2><img src="turtle-marker.png" alt="logo"></img>{t('title')}</h2>
                
                {/* Open Tracker History */}
                <button onClick={() => this.props.setPaneState1({ isPaneOpen: true })}>
                    {t('history')}
                </button>

                {/* Open Settings */}
                <button onClick={() => this.props.setPaneState2({ isPaneOpen: true })}>
                    {t('settings')}
                </button>
            </div>
        )
    }
}

export default withTranslation()(Header);