const styles = theme => ({
    root: {
        display: 'flex',
        flexWrap: 'wrap'
    },
    formControl: {
        margin: theme.spacing.unit,
        minWidth: 120,
        maxWidth: 300
    },
    chips: {
        display: 'flex',
        flexWrap: 'wrap'
    },
    chip: {
        margin: theme.spacing.unit / 4
    },
    noLabel: {
        marginTop: theme.spacing.unit * 3
    }
});

export default styles;
