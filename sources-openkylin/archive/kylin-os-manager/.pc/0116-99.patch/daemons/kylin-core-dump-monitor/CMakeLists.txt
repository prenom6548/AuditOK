project(kylin-core-dump-monitor)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/first-party-service-list.conf DESTINATION /usr/share/kylin-os-manager/kylin-core-dump-monitor)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/first-party-ui-app-list.conf DESTINATION /usr/share/kylin-os-manager/kylin-core-dump-monitor)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-core-dump-handler.sh DESTINATION /usr/share/kylin-os-manager/kylin-core-dump-monitor
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-core-dump-monitor.sh DESTINATION /usr/share/kylin-os-manager/kylin-core-dump-monitor
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-core-dump-trailblazer.sh DESTINATION /usr/share/kylin-os-manager/kylin-core-dump-monitor
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-core-dump-monitor.service DESTINATION /lib/systemd/system/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/kylin-core-dump-trailblazer.service DESTINATION /lib/systemd/system/)