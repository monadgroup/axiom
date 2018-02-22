cmake_minimum_required(VERSION 3.4.3)
project(vst2)

set(SOURCE_FILES public.sdk/source/vst2.x/audioeffect.cpp
                 public.sdk/source/vst2.x/audioeffectx.cpp
                 public.sdk/source/vst2.x/vstplugmain.cpp)

add_library(vst_sdk ${SOURCE_FILES})
