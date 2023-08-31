import * as React from 'react';
import { Translation } from 'react-i18next';
import i18n from "i18next";
import TranslateIcon from '@material-ui/icons/Translate';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import FormHelperText from '@material-ui/core/FormHelperText';
import FormControl from '@material-ui/core/FormControl';
import Select from '@material-ui/core/Select';
import SettingsRemoteIcon from '@material-ui/icons/SettingsRemote';
import Accordion from '@material-ui/core/Accordion';
import AccordionSummary from '@material-ui/core/AccordionSummary';
import AccordionDetails from '@material-ui/core/AccordionDetails';
import Typography from '@material-ui/core/Typography';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import { makeStyles } from '@material-ui/core/styles';
import config from "../config/config"

const useStyles = makeStyles((theme) => ({
  root: {
    width: '100%',
  },
  heading: {
    fontSize: theme.typography.pxToRem(15),
    fontWeight: theme.typography.fontWeightRegular,
  },
}));

function Settings() {
    const [selectedOption, setSelectedOption] = React.useState(() => {
      const saved = localStorage.getItem("i18nextLng").substring(0, 2);
      return saved || "";
    });

    function handleChange (event: React.ChangeEvent<{ value: unknown }>)  {
        i18n.changeLanguage(event.target.value as string);
        setSelectedOption(event.target.value as string);
    };

    const classes = useStyles();

    return (
        <div className={classes.root} style={{ width: '100%', padding: '15px' }}>
        <Accordion defaultExpanded>
            <AccordionSummary
              expandIcon={<ExpandMoreIcon />}
              aria-controls="panel1a-content"
              id="panel1a-header"
            >
              <Typography className={classes.heading}><TranslateIcon /> <Translation>
                        {
                            t => <>{ t('settings.select-language') }</>
                        }
                    </Translation></Typography>
            </AccordionSummary>
            <AccordionDetails>
                <FormControl>
                    <InputLabel id="demo-simple-select-label">Language</InputLabel>
                    <Select
                        id="language-select"
                        value={selectedOption}
                        onChange={handleChange}
                    >
                      <MenuItem value={'en'}>English</MenuItem>
                      <MenuItem value={'fr'}>français</MenuItem>
                      {/*<MenuItem value={'de'}>deutsch</MenuItem>
                      <MenuItem value={'zh'}>简体中文</MenuItem>*/}
                    </Select>
                </FormControl>
            </AccordionDetails>
          </Accordion>

          <Accordion defaultExpanded>
            <AccordionSummary
              expandIcon={<ExpandMoreIcon />}
              aria-controls="panel2a-content"
              id="panel2a-header"
            >
              <Typography className={classes.heading}><SettingsRemoteIcon /> Sensor Configuration Toolkit</Typography>
            </AccordionSummary>
            <AccordionDetails>
                <Typography>
                                <Translation>
                                {
                                    t => <p className="card-text">{ t('about.configmode') }</p>
                                }
                                </Translation>
                                <Translation>
                                {
                                    t => <a href="./configuration/index.html" target="_blank" className="btn btn-mac" style={{ backgroundColor: config.options.mainColor }}>{ t('learn-more') }</a>
                                }
                                </Translation>
                </Typography>
            </AccordionDetails>
        </Accordion>

            {/*<div>
                <div>
                    <div style={{ paddingBottom: '20px' }}>
                        <div className="card shadow bradius" style={{ minHeight: '100%' }}>
                            <div className="card-body">

                <h4><TranslateIcon /> 
                    <Translation>
                        {
                            t => <span style={{ paddingLeft: '5px' }}>{ t('settings.select-language') }</span>
                        }
                    </Translation>
                </h4>

                <FormControl>
                    <InputLabel id="demo-simple-select-label">Language</InputLabel>
                    <Select
                        id="language-select"
                        value={selectedOption}
                        onChange={handleChange}
                    >
                      <MenuItem value={'en'}>English</MenuItem>
                      <MenuItem value={'fr'}>français</MenuItem>
                      <MenuItem value={'de'}>deutsch</MenuItem>
                      <MenuItem value={'zh'}>简体中文</MenuItem>
                    </Select>
                </FormControl>

               
            </div>
                        </div>
                    </div>
                </div>
            </div>
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
                                    t => <a href="./configuration/index.html" target="_blank" className="btn btn-mac" style={{ backgroundColor: config.options.mainColor }}>{ t('learn-more') }</a>
                                }
                                </Translation>
                            </div>
                        </div>
                    </div>
                </div>
            </div>*/}
        </div>
    );
}

export default Settings;
