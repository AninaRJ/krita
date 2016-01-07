set(kritaimpex_LIB_SRCS
    ImpexDebug.cpp
    KisImpExBase.cpp
    KisExportFilter.cpp
    KisExportFilterRegistry.cpp
    KisImportFilter.cpp
    KisImportFilterRegistry.cpp
)

add_library(kritaimpex SHARED ${kritaimpex_LIB_SRCS})
generate_export_header(kritaimpex BASE_NAME kritaimpex)

target_link_libraries(kritaimpex
    PUBLIC
        kritaglobal
        Qt5::Core
)

set_target_properties(kritaimpex PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritaimpex ${INSTALL_TARGETS_DEFAULT_ARGS} )

