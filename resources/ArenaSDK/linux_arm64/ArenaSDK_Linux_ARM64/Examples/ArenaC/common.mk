ARCH_TYPE = $(shell dpkg --print-architecture)

ifeq ($(ARCH_TYPE), amd64)

OPENCV_ROOT_PATH = ../../../OpenCV
    INC_OPENCV_PATH = -I$(OPENCV_ROOT_PATH)/include
    LIB_OPENCV_SHARED = -L$(OPENCV_ROOT_PATH)/lib \
                    -lopencv_calib3d \
                    -lopencv_core \
                    -lopencv_features2d \
                    -lopencv_highgui \
                    -lopencv_imgproc \
                    -lopencv_video

LDFLAGS = -L../../../lib64 \
          -L../../../GenICam/library/lib/Linux64_x64 \
          -L../../../ffmpeg \
          $(LIB_OPENCV_SHARED)

GENICAMLIBS = -lGCBase_gcc54_v3_3_LUCID \
              -lGenApi_gcc54_v3_3_LUCID \
              -lLog_gcc54_v3_3_LUCID \
              -llog4cpp_gcc54_v3_3_LUCID \
              -lMathParser_gcc54_v3_3_LUCID \
              -lNodeMapData_gcc54_v3_3_LUCID \
              -lXmlParser_gcc54_v3_3_LUCID

OUTDIR = ../../../OutputDirectory/Linux/x64Release/

else ifeq ($(ARCH_TYPE), armhf)

LDFLAGS = -L../../../lib \
          -L../../../GenICam/library/lib/Linux32_ARMhf \
          -L../../../ffmpeg \
          $(LIB_OPENCV_SHARED)

GENICAMLIBS = -lGCBase_gcc540_v3_3_LUCID \
              -lGenApi_gcc540_v3_3_LUCID \
              -lLog_gcc540_v3_3_LUCID \
              -llog4cpp_gcc540_v3_3_LUCID \
              -lMathParser_gcc540_v3_3_LUCID \
              -lNodeMapData_gcc540_v3_3_LUCID \
              -lXmlParser_gcc540_v3_3_LUCID


OUTDIR = ../../../OutputDirectory/armhf/x32Release/
else ifeq ($(ARCH_TYPE), arm64)

LDFLAGS = -L../../../lib \
          -L../../../GenICam/library/lib/Linux64_ARM \
          -L../../../ffmpeg \
          $(LIB_OPENCV_SHARED)

GENICAMLIBS = -lGCBase_gcc54_v3_3_LUCID \
              -lGenApi_gcc54_v3_3_LUCID \
              -lLog_gcc54_v3_3_LUCID \
              -llog4cpp_gcc54_v3_3_LUCID \
              -lMathParser_gcc54_v3_3_LUCID \
              -lNodeMapData_gcc54_v3_3_LUCID \
              -lXmlParser_gcc54_v3_3_LUCID


OUTDIR = ../../../OutputDirectory/arm64/x64Release/
endif


CC=g++

INCLUDE = -I../../../include/ArenaC \
          -I../../../include/SaveC \
          $(INC_OPENCV_PATH)

CFLAGS=-Wall -g -O2 -Wno-unknown-pragmas

FFMPEGLIBS = -lavcodec \
             -lavformat \
             -lavutil \
             -lswresample

LIBS = -larenac \
       -larena \
       -lsavec \
       -lsave \
       -lgentl \
       -lpthread \
       $(GENICAMLIBS) \
       $(FFMPEGLIBS) \
       $(LIB_OPENCV_SHARED)

RM = rm -f

SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
DEPS = $(OBJS:%.o=%.d)

.PHONY: all
all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} $^ -o $@ $(LIBS)
	-mkdir -p $(OUTDIR)
	-cp $(TARGET) $(OUTDIR)

%.o: %.c ${SRCS}
	${CC} ${INCLUDE}  ${LDFLAGS} ${LIBS} -o $@ $< -c ${CFLAGS}

${DEPS}: %.c
	${CC} ${CFLAGS} ${INCLUDE} -MM $< >$@

-include $(OBJS:.o=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET} ${OBJS} ${DEPS}
