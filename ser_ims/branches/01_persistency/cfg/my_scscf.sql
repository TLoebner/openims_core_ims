CREATE TABLE auth_data_bulk (
    snapshot_version INT NOT NULL,
    step_version INT NOT NULL,
    data BLOB,
    PRIMARY KEY (snapshot_version, step_version)
);

CREATE TABLE auth_data_cache (
    snapshot_version INT NOT NULL,
    step_version INT NOT NULL,
    private VARCHAR(80) NOT NULL,
    public  VARCHAR(128) NOT NULL,
    data BLOB,
    PRIMARY KEY (snapshot_version, step_version, private, public)
);

CREATE TABLE dialogs_bulk (
    snapshot_version INT NOT NULL,
    step_version INT NOT NULL,
    data BLOB,
    PRIMARY KEY (snapshot_version, step_version)
);

CREATE TABLE registrar_bulk (
    snapshot_version INT NOT NULL,
    step_version INT NOT NULL,
    data BLOB,
    PRIMARY KEY (snapshot_version, step_version)
);