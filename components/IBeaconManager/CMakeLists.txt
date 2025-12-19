idf_component_register(
        SRCS "src/IBeaconManager.cpp"
        INCLUDE_DIRS "include"
        REQUIRES bt nvs_flash
)