
########### next target ###############

set(konq_genhash_PART_SRCS genhash.cpp )


kde4_add_plugin(konq_genhash ${konq_genhash_PART_SRCS})


target_link_libraries(konq_genhash ${KDE4_KPARTS_LIBS} ${KDE4_KDESU_LIBS})

install(TARGETS konq_genhash  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############
install( FILES genhash.rc genhash.desktop  DESTINATION  ${DATA_INSTALL_DIR}/dolphinpart/kpartplugins )
