# Install script for directory: /home/docker/project/ns-3-dev/src/core

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so"
         RPATH "/usr/local/lib:\$ORIGIN/:\$ORIGIN/../lib:/usr/local/lib64:\$ORIGIN/:\$ORIGIN/../lib64")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/docker/project/ns-3-dev/build/lib/libns3.46-core-optimized.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so"
         OLD_RPATH "/home/docker/project/ns-3-dev/build/lib::::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH "/usr/local/lib:\$ORIGIN/:\$ORIGIN/../lib:/usr/local/lib64:\$ORIGIN/:\$ORIGIN/../lib64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libns3.46-core-optimized.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ns3" TYPE FILE FILES
    "/home/docker/project/ns-3-dev/build/include/ns3/core-config.h"
    "/home/docker/project/ns-3-dev/src/core/model/int64x64-128.h"
    "/home/docker/project/ns-3-dev/src/core/helper/csv-reader.h"
    "/home/docker/project/ns-3-dev/src/core/helper/event-garbage-collector.h"
    "/home/docker/project/ns-3-dev/src/core/helper/random-variable-stream-helper.h"
    "/home/docker/project/ns-3-dev/src/core/model/abort.h"
    "/home/docker/project/ns-3-dev/src/core/model/ascii-file.h"
    "/home/docker/project/ns-3-dev/src/core/model/ascii-test.h"
    "/home/docker/project/ns-3-dev/src/core/model/assert.h"
    "/home/docker/project/ns-3-dev/src/core/model/attribute-accessor-helper.h"
    "/home/docker/project/ns-3-dev/src/core/model/attribute-construction-list.h"
    "/home/docker/project/ns-3-dev/src/core/model/attribute-container.h"
    "/home/docker/project/ns-3-dev/src/core/model/attribute-helper.h"
    "/home/docker/project/ns-3-dev/src/core/model/attribute.h"
    "/home/docker/project/ns-3-dev/src/core/model/boolean.h"
    "/home/docker/project/ns-3-dev/src/core/model/breakpoint.h"
    "/home/docker/project/ns-3-dev/src/core/model/build-profile.h"
    "/home/docker/project/ns-3-dev/src/core/model/calendar-scheduler.h"
    "/home/docker/project/ns-3-dev/src/core/model/callback.h"
    "/home/docker/project/ns-3-dev/src/core/model/command-line.h"
    "/home/docker/project/ns-3-dev/src/core/model/config.h"
    "/home/docker/project/ns-3-dev/src/core/model/default-deleter.h"
    "/home/docker/project/ns-3-dev/src/core/model/default-simulator-impl.h"
    "/home/docker/project/ns-3-dev/src/core/model/demangle.h"
    "/home/docker/project/ns-3-dev/src/core/model/deprecated.h"
    "/home/docker/project/ns-3-dev/src/core/model/des-metrics.h"
    "/home/docker/project/ns-3-dev/src/core/model/double.h"
    "/home/docker/project/ns-3-dev/src/core/model/enum.h"
    "/home/docker/project/ns-3-dev/src/core/model/event-id.h"
    "/home/docker/project/ns-3-dev/src/core/model/event-impl.h"
    "/home/docker/project/ns-3-dev/src/core/model/fatal-error.h"
    "/home/docker/project/ns-3-dev/src/core/model/fatal-impl.h"
    "/home/docker/project/ns-3-dev/src/core/model/fd-reader.h"
    "/home/docker/project/ns-3-dev/src/core/model/environment-variable.h"
    "/home/docker/project/ns-3-dev/src/core/model/global-value.h"
    "/home/docker/project/ns-3-dev/src/core/model/hash-fnv.h"
    "/home/docker/project/ns-3-dev/src/core/model/hash-function.h"
    "/home/docker/project/ns-3-dev/src/core/model/hash-murmur3.h"
    "/home/docker/project/ns-3-dev/src/core/model/hash.h"
    "/home/docker/project/ns-3-dev/src/core/model/heap-scheduler.h"
    "/home/docker/project/ns-3-dev/src/core/model/int64x64-double.h"
    "/home/docker/project/ns-3-dev/src/core/model/int64x64.h"
    "/home/docker/project/ns-3-dev/src/core/model/integer.h"
    "/home/docker/project/ns-3-dev/src/core/model/length.h"
    "/home/docker/project/ns-3-dev/src/core/model/list-scheduler.h"
    "/home/docker/project/ns-3-dev/src/core/model/log-macros-disabled.h"
    "/home/docker/project/ns-3-dev/src/core/model/log-macros-enabled.h"
    "/home/docker/project/ns-3-dev/src/core/model/log.h"
    "/home/docker/project/ns-3-dev/src/core/model/make-event.h"
    "/home/docker/project/ns-3-dev/src/core/model/map-scheduler.h"
    "/home/docker/project/ns-3-dev/src/core/model/math.h"
    "/home/docker/project/ns-3-dev/src/core/model/names.h"
    "/home/docker/project/ns-3-dev/src/core/model/node-printer.h"
    "/home/docker/project/ns-3-dev/src/core/model/nstime.h"
    "/home/docker/project/ns-3-dev/src/core/model/object-base.h"
    "/home/docker/project/ns-3-dev/src/core/model/object-factory.h"
    "/home/docker/project/ns-3-dev/src/core/model/object-map.h"
    "/home/docker/project/ns-3-dev/src/core/model/object-ptr-container.h"
    "/home/docker/project/ns-3-dev/src/core/model/object-vector.h"
    "/home/docker/project/ns-3-dev/src/core/model/object.h"
    "/home/docker/project/ns-3-dev/src/core/model/pair.h"
    "/home/docker/project/ns-3-dev/src/core/model/pointer.h"
    "/home/docker/project/ns-3-dev/src/core/model/priority-queue-scheduler.h"
    "/home/docker/project/ns-3-dev/src/core/model/ptr.h"
    "/home/docker/project/ns-3-dev/src/core/model/random-variable-stream.h"
    "/home/docker/project/ns-3-dev/src/core/model/rng-seed-manager.h"
    "/home/docker/project/ns-3-dev/src/core/model/rng-stream.h"
    "/home/docker/project/ns-3-dev/src/core/model/scheduler.h"
    "/home/docker/project/ns-3-dev/src/core/model/show-progress.h"
    "/home/docker/project/ns-3-dev/src/core/model/shuffle.h"
    "/home/docker/project/ns-3-dev/src/core/model/simple-ref-count.h"
    "/home/docker/project/ns-3-dev/src/core/model/simulation-singleton.h"
    "/home/docker/project/ns-3-dev/src/core/model/simulator-impl.h"
    "/home/docker/project/ns-3-dev/src/core/model/simulator.h"
    "/home/docker/project/ns-3-dev/src/core/model/singleton.h"
    "/home/docker/project/ns-3-dev/src/core/model/string.h"
    "/home/docker/project/ns-3-dev/src/core/model/synchronizer.h"
    "/home/docker/project/ns-3-dev/src/core/model/system-path.h"
    "/home/docker/project/ns-3-dev/src/core/model/system-wall-clock-ms.h"
    "/home/docker/project/ns-3-dev/src/core/model/system-wall-clock-timestamp.h"
    "/home/docker/project/ns-3-dev/src/core/model/test.h"
    "/home/docker/project/ns-3-dev/src/core/model/time-printer.h"
    "/home/docker/project/ns-3-dev/src/core/model/timer-impl.h"
    "/home/docker/project/ns-3-dev/src/core/model/timer.h"
    "/home/docker/project/ns-3-dev/src/core/model/trace-source-accessor.h"
    "/home/docker/project/ns-3-dev/src/core/model/traced-callback.h"
    "/home/docker/project/ns-3-dev/src/core/model/traced-value.h"
    "/home/docker/project/ns-3-dev/src/core/model/trickle-timer.h"
    "/home/docker/project/ns-3-dev/src/core/model/tuple.h"
    "/home/docker/project/ns-3-dev/src/core/model/type-id.h"
    "/home/docker/project/ns-3-dev/src/core/model/type-name.h"
    "/home/docker/project/ns-3-dev/src/core/model/type-traits.h"
    "/home/docker/project/ns-3-dev/src/core/model/uinteger.h"
    "/home/docker/project/ns-3-dev/src/core/model/uniform-random-bit-generator.h"
    "/home/docker/project/ns-3-dev/src/core/model/valgrind.h"
    "/home/docker/project/ns-3-dev/src/core/model/vector.h"
    "/home/docker/project/ns-3-dev/src/core/model/warnings.h"
    "/home/docker/project/ns-3-dev/src/core/model/watchdog.h"
    "/home/docker/project/ns-3-dev/src/core/model/realtime-simulator-impl.h"
    "/home/docker/project/ns-3-dev/src/core/model/wall-clock-synchronizer.h"
    "/home/docker/project/ns-3-dev/src/core/model/val-array.h"
    "/home/docker/project/ns-3-dev/src/core/model/matrix-array.h"
    "/home/docker/project/ns-3-dev/build/include/ns3/core-module.h"
    "/home/docker/project/ns-3-dev/build/include/ns3/core-export.h"
    )
endif()

