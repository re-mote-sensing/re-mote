import * as React from 'react';
import { Translation } from 'react-i18next';
import { Timeline } from 'react-twitter-widgets'
import config from "../config/config"

function Info() {
    return (
        <div style={{ width: '100%', padding: '15px' }}>

            <img
                className="img-waterflowingImage shadow bradius" 
                src="./images/waterflowingnew.gif"
                style={{ height: 'auto', width: '100%' }}
                alt="waterflowingImage"
            />
            <br />
            <br />
            <div>
                <div className="row">
                    <div className="col-md-4 col-xs-12" style={{ paddingBottom: '20px' }}>
                        <div className="card shadow bradius" style={{ minHeight: '100%' }}>
                            <img 
                                className="card-img-top" 
                                src="./images/sixnations.svg" 
                                alt="sixnationslogo" 
                                style={{ 
                                    width: '80%', 
                                    height: 'auto', 
                                    marginLeft: 'auto', 
                                    marginRight: 'auto', 
                                    marginTop: '10px' 
                                }} 
                            />
                            <div className="card-body">
                                <h4 className="card-title">SIX NATIONS</h4>
                                <p>OF THE GRAND RIVER</p>
                                <Translation>
                                {
                                    t => <p className="card-text">{ t('about.sixnations') }</p>
                                }
                                </Translation>
                                <Translation>
                                {
                                    t => <a href="https://www.sixnations.ca/about" target="_blank" className="btn btn-mac" style={{ backgroundColor: config.options.mainColor }}>{ t('learn-more') }</a>
                                }
                                </Translation>
                            </div>
                        </div>
                    </div>

                    <div className="col-md-4 col-xs-12" style={{ paddingBottom: '20px' }}>
                        <div className="card shadow bradius" style={{ minHeight: '100%' }}>
                            <img 
                                className="card-img-top" 
                                src="./images/ohneganos-new+watermark.png" 
                                alt="maclogo" 
                                style={{ 
                                    width: '50%', 
                                    height: 'auto', 
                                    marginLeft: 'auto', 
                                    marginRight: 'auto', 
                                    marginTop: '10px' 
                                }} 
                            />
                            <div className="card-body">
                                <h4 className="card-title">OHNEGANOS</h4>
                                <Translation>
                                {
                                    t => <p className="card-text">{ t('about.ohneganos') }</p>
                                }
                                </Translation>
                                <Translation>
                                {
                                    t => <a href="https://www.ohneganos.com" target="_blank" className="btn btn-mac" style={{ backgroundColor: config.options.mainColor }}>{ t('learn-more') }</a>
                                }
                                </Translation>
                            </div>
                        </div>
                    </div>

                    {/*<div className="col-md-4 col-xs-12" style={{ paddingBottom: '20px' }}>
                        <div className="card shadow bradius" style={{ minHeight: '100%' }}>
                            <img 
                                className="card-img-top" 
                                src="./images/maclogo.svg" 
                                alt="maclogo" 
                                style={{ 
                                    width: '80%', 
                                    height: 'auto', 
                                    marginLeft: 'auto', 
                                    marginRight: 'auto', 
                                    marginTop: '10px' 
                                }} 
                            />
                            <div className="card-body">
                                <h4 className="card-title">MacWater</h4>
                                <Translation>
                                {
                                    t => <p className="card-text">{ t('about.macwater') }</p>
                                }
                                </Translation>
                                <Translation>
                                {
                                    t => <a href="http://www.macwater.org/" target="_blank" className="btn btn-mac" style={{ backgroundColor: config.options.mainColor }}>{ t('learn-more') }</a>
                                }
                                </Translation>
                            </div>
                        </div>
                    </div>*/}

                    <div className="col-md-4 col-xs-12" style={{ paddingBottom: '20px' }}>
                        <div className="card shadow bradius" style={{ minHeight: '100%' }}>
                            <img
                                className="card-img-top"
                                src="./images/GWFlogo.png"
                                alt="GWFlogo"
                                style={{
                                    width: '50%',
                                    height: 'auto',
                                    marginLeft: 'auto',
                                    marginRight: 'auto',
                                    marginTop: '10px'
                                }}
                            />
                            <div className="card-body">
                                <h4 className="card-title">Global Water Futures</h4>
                                <Translation>
                                {
                                    t => <p className="card-text">{ t('about.gwf') }</p>
                                }
                                </Translation>
                                <Translation>
                                {
                                    t => <a href="https://gwf.usask.ca/about-contact/about-us.php" target="_blank" className="btn btn-mac" style={{ backgroundColor: config.options.mainColor }}>{ t('learn-more') }</a>
                                }
                                </Translation>
                            </div>
                        </div>
                    </div>
                </div>

                <div className="row">
                    <div className="col-md-4 col-xs-12">
                        <div className="card shadow bradius" style={{ maxHeight: '100%' }}>
                            <div className="card-body" style={{ padding: '0px' }}>
                                <Timeline
                                    dataSource={{ sourceType: "url", url: "https://twitter.com/SixNationsGR" }}
                                    options={{ width: "100%", height: "600", dnt: "true" }}
                                />
                            </div>
                        </div>
                    </div>
                    <div className="col-md-4 col-xs-12">
                        <div className="card shadow bradius" style={{ maxHeight: '100%' }}>
                            <div className="card-body" style={{ padding: '0px' }}>
                                <Timeline
                                    dataSource={{ sourceType: "url", url: "https://twitter.com/ohnegahde" }}
                                    options={{ width: "100%", height: "600", dnt: "true" }}
                                />
                            </div>
                        </div>
                    </div>
                    {/*<div className="col-md-4 col-xs-12">
                        <div className="card shadow bradius" style={{ maxHeight: '100%' }}>
                            <div className="card-body" style={{ padding: '0px' }}>
                                <Timeline
                                    dataSource={{ sourceType: "url", url: "https://twitter.com/mcmasteru" }}
                                    options={{ width: "100%", height: "600", dnt: "true" }}
                                />
                            </div>
                        </div>
                    </div>*/}
                    <div className="col-md-4 col-xs-12">
                        <div className="card shadow bradius" style={{ maxHeight: '100%' }}>
                            <div className="card-body" style={{ padding: '0px' }}>
                                <Timeline
                                    dataSource={{ sourceType: "url", url: "https://twitter.com/GWFutures?ref_src=twsrc%5Etfw" }}
                                    options={{ width: "100%", height: "600", dnt: "true" }}
                                />
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}

export default Info;
