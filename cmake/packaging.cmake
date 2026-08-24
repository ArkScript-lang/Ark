set(CPACK_PACKAGE_NAME "arkscript")
set(CPACK_PACKAGE_VERSION "${ARK_VERSION_STR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A small, lisp-inspired, functional scripting language")
set(CPACK_PACKAGE_VENDOR "ArkScript")
set(CPACK_PACKAGE_CONTACT "contact@arkscript-lang.dev")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/.github/images/logo-2026-rectangle.png")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}")
set(CPACK_OUTPUT_FILE_PREFIX "${PROJECT_BINARY_DIR}/packages")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENCE")

# === Debian ===
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "contact@arkscript-lang.dev")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")  # Focuses on scripting/automation
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# === rpm ===
set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
set(CPACK_RPM_PACKAGE_LICENSE "MPL2.0")

# === macOS DragNDrop ===
set(CPACK_PACKAGE_EXECUTABLES "arkscript")

# === AUR PKGBUILD ===
configure_file(
    ${CMAKE_SOURCE_DIR}/packaging/PKGBUILD-src.in
    ${CMAKE_BINARY_DIR}/packages/PKGBUILD-src
    @ONLY
)
configure_file(
    ${CMAKE_SOURCE_DIR}/packaging/PKGBUILD-bin.in
    ${CMAKE_BINARY_DIR}/packages/PKGBUILD-bin
    @ONLY
)
