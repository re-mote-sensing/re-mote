import CssBaseline from '@material-ui/core/CssBaseline';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemIcon from '@material-ui/core/ListItemIcon';
import MenuIcon from '@material-ui/core/Menu';
import ListItemText from '@material-ui/core/ListItemText';
import { withStyles } from '@material-ui/core/styles';
import MapIcon from '@material-ui/icons/Map';
import SettingsIcon from '@material-ui/icons/Settings';
import classNames from 'classnames';
import * as PropTypes from 'prop-types';
import * as React from 'react';
import { Link, Route, Switch } from 'react-router-dom';
import Map from '../components/Map';
import Settings from '../components/Settings';
import RealTime from '../components/RealTime';
// import ConfigMode from '../components/ConfigMode';
// import SettingsRemoteIcon from '@material-ui/icons/SettingsRemote';
import Info from '../components/Info';
import InfoIcon from '@material-ui/icons/Info';
import dashboardStyle from '../style/dashboard-style';
import { AppBar, Toolbar, IconButton, Divider, Typography, Hidden, Drawer } from '@material-ui/core';
import { Menu } from '@material-ui/icons';
import { Translation } from 'react-i18next';
import config from "../config/config"
import UpdateIcon from '@material-ui/icons/Update';

class Dashboard extends React.Component<any, any> {
    state = {
        mobileOpen: false,
    };

    handleDrawerToggle = () => {
        this.setState(state => ({ mobileOpen: !state.mobileOpen }));
    };

    render() {
        const { classes } = this.props;
        const { mobileOpen } = this.state;

        const drawer = (
            <div>
                <Hidden xsDown>
                    <div className={classes.toolbar} style={{ backgroundColor: config.options.mainColor }}>
                        {/*<div style={{ paddingTop: '20px', marginLeft: '24px', fontSize: '20px', color:'#fff' }}>
                            re:mote
                        </div>*/}
                        <div style={{ paddingTop: '20px', marginLeft: '24px', fontSize: '16px', color:'#fff' }}>
                            {config.options.subTitle}
                        </div>
                    </div>
                </Hidden>
                <Divider />
                <List>
                    <Link to="/Map">
                        <ListItem button key="map">
                            <ListItemIcon>
                                <MapIcon />
                            </ListItemIcon>
                            <Translation>
                            {
                                (t) => <ListItemText primary={t('dashboard.map')} />
                            }
                            </Translation>
                        </ListItem>
                    </Link>
                    <Link to="/RealTime">
                        <ListItem button key="realtime">
                            <ListItemIcon>
                                <UpdateIcon />
                            </ListItemIcon>
                            <ListItemText primary="Real-time" />
                        </ListItem>
                    </Link>
                    <Link to="/Info">
                        <ListItem button key="info">
                            <ListItemIcon>
                                <InfoIcon />
                            </ListItemIcon>
                            <Translation>
                            {
                                (t) => <ListItemText primary={t('dashboard.info')} />
                            }
                            </Translation>
                        </ListItem>
                    </Link>
                    <Divider />
                    {/*<Link to="/ConfigMode">
                        <ListItem button key="configmode">
                            <ListItemIcon>
                                <SettingsRemoteIcon />
                            </ListItemIcon>
                            <Translation>
                            {
                                (t) => <ListItemText primary={t('dashboard.configmode')} />
                            }
                            </Translation>
                        </ListItem>
                    </Link>*/}
                    <Link to="/Settings">
                        <ListItem button key="settings">
                            <ListItemIcon>
                                <SettingsIcon />
                            </ListItemIcon>
                            <Translation>
                            {
                                (t) => <ListItemText primary={t('dashboard.settings')} />
                            }
                            </Translation>
                        </ListItem>
                    </Link>
                </List>
            </div>
        );

        return (<React.Fragment>
            <div className={classes.root}>
                <CssBaseline />
                <AppBar position="absolute" className={classes.appBar} style={{ backgroundColor: config.options.mainColor }}>
                    <Toolbar>
                        <IconButton
                            color="inherit"
                            aria-label="Open drawer"
                            onClick={this.handleDrawerToggle}
                            className={classes.menuButton}
                        >
                            <Menu />
                        </IconButton>
                        <Typography variant="h6" color="inherit" noWrap>
                        <img
                            src={config.options.logo}
                            style={{ height: '50px', width: 'auto', marginRight:'20px' }}
                            alt=""
                        />
                            {config.options.title}
                        </Typography>
                    </Toolbar>
                </AppBar>
                <nav className={classes.drawer}>
                    {/* The implementation can be swapped with js to avoid SEO duplication of links. */}
                    <Hidden smUp implementation="css">
                        <Drawer
                            container={this.props.container}
                            variant="temporary"
                            open={mobileOpen}
                            onClose={this.handleDrawerToggle}
                            classes={{
                                paper: classes.drawerPaper,
                            }}
                        >
                            {drawer}
                        </Drawer>
                    </Hidden>
                    <Hidden xsDown implementation="css">
                        <Drawer
                            classes={{
                                paper: classes.drawerPaper,
                            }}
                            variant="permanent"
                            open
                        >
                            {drawer}
                        </Drawer>
                    </Hidden>
                </nav>
                <main className={classes.content}>
                    <div className={classes.toolbar} />
                    <Switch>
                        <Route path="/Map" component={Map} />
                        <Route path="/RealTime" component={RealTime} />
                        <Route path="/Info" component={Info} />
                        <Route path="/Settings" component={Settings} />
                        {/*<Route path="/ConfigMode" component={ConfigMode} />*/}
                        <Route component={Map} />
                    </Switch>
                </main>
            </div>
        </React.Fragment>
        );
    }
}

(Dashboard as any).propTypes = {
    classes: PropTypes.object.isRequired
};

export default withStyles(dashboardStyle as any)(Dashboard);
