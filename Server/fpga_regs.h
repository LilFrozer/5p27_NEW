
#pragma once

#include "definesMil.h"

namespace fpga_regs
{

const u16 SHIFT_DIR_TX = 0x8;
const u16 FPGAREG_FARBOS = 0x450;  // addr "Контроллер ФАР-БОС"
const u16 FPGAREG_FARBOS_CNTR = 0x7 + FPGAREG_FARBOS;
const u16 FPGAREG_FARBOS_DIR_TX = SHIFT_DIR_TX + FPGAREG_FARBOS;

struct summ_reg_dir_rx
{
    u32 ch_id : 4;
    u32 dlid : 2;
    u32 dpid : 2;
    u32 dmid : 4;
    u32 : 20;
};

struct summ_reg_dir_tx
{
    u32 dir_tx :4;
    u32 : 28;
};

struct summ_reg_ctrl_sum
{
    u32 ctrl_sum : 2;
    u32 : 30;
};

struct summ_reg_mask
{
    u32 mask : 5;
    u32 : 27;
};

struct fpga_mem_summator
{
    u32 status[7]{};
    u32 cntr{};
    summ_reg_dir_tx dir_tx[7]{};
    summ_reg_ctrl_sum cntr_sum{};
    summ_reg_mask mask[3]{};
    summ_reg_dir_rx dir_rx_az{};
    summ_reg_dir_rx dir_rx_css{};
    summ_reg_dir_rx dir_rx[5]{};
    summ_reg_dir_rx dir_rx_sum1[5]{};
    summ_reg_dir_rx dir_rx_sum2[5]{};
    summ_reg_dir_rx dir_rx_sum3[5]{};
    u32 mask_devices{};
};

const u32 amount_regs_summator = sizeof(fpga_mem_summator) / sizeof(u32);

}
