import * as React from 'react';
import { Translation } from 'react-i18next';
import i18n from "i18next";
import SettingsRemoteIcon from '@material-ui/icons/SettingsRemote';

function ConfigMode() {

    return (
        <div style={{ width: '100%', padding: '15px' }}>
            <div>
                <div>
                    <div style={{ paddingBottom: '20px' }}>
                        <div className="card shadow bradius" style={{ minHeight: '100%' }}>
                            <div className="card-body">
                                <h4 className="card-title"><SettingsRemoteIcon /> Sensor Configuration Toolkit</h4>
                                <Translation>
                                {
                                    t => <p className="card-text">{ t('about.configmode') }</p>
                                }
                                </Translation>
                                <Translation>
                                {
                                    t => <a href="./configuration/index.html" target="_blank" className="btn btn-mac">{ t('learn-more') }</a>
                                }
                                </Translation>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}

export default ConfigMode;
