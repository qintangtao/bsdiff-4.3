LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := bsdiff
LOCAL_SRC_FILES :=  bsdiff.c xmd5.c \
					../../bzip2-1.0.6/blocksort.c \
					../../bzip2-1.0.6/bzlib.c \
					../../bzip2-1.0.6/compress.c \
					../../bzip2-1.0.6/decompress.c \
					../../bzip2-1.0.6/crctable.c \
					../../bzip2-1.0.6/randtable.c \
					../../bzip2-1.0.6/huffman.c
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := bspatch
LOCAL_SRC_FILES :=  bspatch.c xmd5.c \
					../../bzip2-1.0.6/blocksort.c \
					../../bzip2-1.0.6/bzlib.c \
					../../bzip2-1.0.6/compress.c \
					../../bzip2-1.0.6/decompress.c \
					../../bzip2-1.0.6/crctable.c \
					../../bzip2-1.0.6/randtable.c \
					../../bzip2-1.0.6/huffman.c
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)