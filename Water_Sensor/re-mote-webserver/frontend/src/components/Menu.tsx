import FormControl from '@material-ui/core/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import Select from '@material-ui/core/Select';
import * as React from 'react';
import { Location } from '~/components/Map';
import { Translation } from 'react-i18next';

interface MenuProps {
    classes: any,
    selectedType: string,
    selectedLocation: Location,
    locations: Location[][],
    types: string[][],
    changeLocation: (location: Location) => void,
    changeType: (type: string) => void
}

class Menu extends React.PureComponent<MenuProps, null> {
    constructor(props) {
        super(props);
    }

    handleTypeChange: React.ChangeEventHandler<HTMLSelectElement> = event => {
        const { types } = this.props;
        var type = event.target.value
        for (var i = 0; i < types[1].length; i++) {
            if (type == types[1][i]) {
                this.props.changeLocation(null);
                break;
            }
        }
        this.props.changeType(type);
    };

    handleLocationChange = event => {
        var location = event.target.value;
        if (location == "") {
            this.props.changeLocation(null);
        } else {
            const { locations } = this.props;
            for (var i = 0; i < locations[0].length; i++) {
                if (locations[0][i].id == location) {
                    this.props.changeLocation(locations[0][i]);
                    return;
                }
            }
            for (var i = 0; i < locations[1].length; i++) {
                if (locations[1][i].id == location) {
                    this.props.changeType("");
                    this.props.changeLocation(locations[1][i]);
                    break;
                }
            }
        }
    };

    render(): React.ReactNode {
        const { classes, selectedType, selectedLocation, locations, types } = this.props;

        return (
            <form className={classes.root}>
                <FormControl className={classes.formControl}>
                    <Translation>
                        {
                            t => <InputLabel htmlFor="select-location">{t('menu.location')}</InputLabel>
                        }
                    </Translation>
                    <Select
                        value={((selectedLocation == null) ? ("") : (selectedLocation.id))}
                        onChange={this.handleLocationChange}
                        inputProps={{
                            name: 'location',
                            id: 'select-location'
                        }}>
                        <MenuItem value="">
                            <Translation>
                                {
                                    t => <em>{t('menu.any')}</em>
                                }
                            </Translation>
                        </MenuItem>

                        {locations[0].map(loc => (
                            <MenuItem key={loc.id} value={loc.id}>{loc.name.replace("_"," ").replace("_", " ")}</MenuItem>
                        ))}

                        {locations[1].map(loc => (
                            <MenuItem key={loc.id} value={loc.id}>{loc.name.replace("_"," ").replace("_", " ")}</MenuItem>
                        ))}

                        {/*{locations[1].map(loc => (
                            <MenuItem style={{ color: 'Silver' }} key={loc.id} value={loc.id}>{loc.name}</MenuItem>
                        ))}*/}
                    </Select>
                </FormControl>
                <FormControl className={classes.formControl}>
                    <Translation>
                        {
                            t => <InputLabel htmlFor="select-sensor-type">{t('menu.sensor-type')}</InputLabel>
                        }
                    </Translation>
                    <Select
                        value={selectedType}
                        onChange={this.handleTypeChange}
                        inputProps={{
                            name: 'sensor-type',
                            id: 'select-sensor-type'
                        }}>
                        <MenuItem value="">
                            <Translation>
                                {
                                    t => <em>{t('menu.any')}</em>
                                }
                            </Translation>
                        </MenuItem>
                        {types[0].map(type => (
                            <MenuItem key={type} value={type}>{type.replace("_"," ").replace("_", " ")}</MenuItem>
                        ))}

                        {types[1].map(type => (
                            <MenuItem style={{ color: 'Silver' }} key={type} value={type}>{type}</MenuItem>
                        ))}
                    </Select>
                </FormControl>
            </form>
        )
    }
}

export default Menu;