; $MODE = "UniformRegister"

; $SPI_VS_OUT_CONFIG.VS_EXPORT_COUNT = 0
; $NUM_SPI_VS_OUT_ID = 1
; out_colour
; $SPI_VS_OUT_ID[0].SEMANTIC_0 = 0

; C0
; $UNIFORM_VARS[0].name = "mvp"
; $UNIFORM_VARS[0].type = "mat4"
; $UNIFORM_VARS[0].count = 1
; $UNIFORM_VARS[0].block = -1
; $UNIFORM_VARS[0].offset = 0

; R1
; $ATTRIB_VARS[0].name = "in_pos"
; $ATTRIB_VARS[0].type = "vec3"
; $ATTRIB_VARS[0].location = 0
; R2
; $ATTRIB_VARS[1].name = "in_col"
; $ATTRIB_VARS[1].type = "vec4"
; $ATTRIB_VARS[1].location = 1

; --------  Disassembly --------------------
00 CALL_FS NO_BARRIER 
01 ALU: ADDR(32) CNT(18)
      0  x: MUL         ____,  C3.w,  1.0f      
         y: MUL         ____,  C3.z,  1.0f      
         z: MUL         ____,  C3.y,  1.0f      
         w: MUL         ____,  C3.x,  1.0f      
      1  x: MULADD      R127.x,  R1.z,  C2.w,  PV0.x      
         y: MULADD      R127.y,  R1.z,  C2.z,  PV0.y      
         z: MULADD      R127.z,  R1.z,  C2.y,  PV0.z      
         w: MULADD      R127.w,  R1.z,  C2.x,  PV0.w      
      2  x: MULADD      R127.x,  R1.y,  C1.w,  PV1.x      
         y: MULADD      R127.y,  R1.y,  C1.z,  PV1.y      
         z: MULADD      R127.z,  R1.y,  C1.y,  PV1.z      
         w: MULADD      R127.w,  R1.y,  C1.x,  PV1.w      
      3  x: MULADD      R1.x,  R1.x,  C0.x,  PV2.w      
         y: MULADD      R1.y,  R1.x,  C0.y,  PV2.z      
         z: MULADD      R1.z,  R1.x,  C0.z,  PV2.y      
         w: MULADD      R1.w,  R1.x,  C0.w,  PV2.x      
02 EXP_DONE: POS0, R1
03 EXP_DONE: PARAM0, R2  NO_BARRIER 
END_OF_PROGRAM

