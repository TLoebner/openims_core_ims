CREATE TABLE auth_data_bulk (
    snapshot_version INTEGER NOT NULL,
    step_version INTEGER NOT NULL,
    data BYTEA NOT NULL,
    PRIMARY KEY (snapshot_version, step_version)
);

CREATE TABLE auth_data_cache (
    snapshot_version INTEGER NOT NULL,
    step_version INTEGER NOT NULL,
    private VARCHAR(80) NOT NULL,
    public  VARCHAR(128) NOT NULL,
    data BYTEA NOT NULL,
    PRIMARY KEY (snapshot_version, step_version, private, public)
);

CREATE TABLE dialogs_bulk (
    snapshot_version INTEGER NOT NULL,
    step_version INTEGER NOT NULL,
    data BYTEA NOT NULL,
    PRIMARY KEY (snapshot_version, step_version)
);

CREATE TABLE registrar_bulk (
    snapshot_version INTEGER NOT NULL,
    step_version INTEGER NOT NULL,
    data BYTEA NOT NULL,
    PRIMARY KEY (snapshot_version, step_version)
);

