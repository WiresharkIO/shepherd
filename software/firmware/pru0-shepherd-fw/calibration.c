#include "calibration.h"
#include "math64_safe.h"

static const volatile struct CalibrationConfig *cal;

void calibration_initialize(const volatile struct CalibrationConfig *const config)
{
	cal = config;
}


/* bring values into adc domain with -> voltage_uV = adc_value * gain_factor + offset
 * original definition in: https://github.com/geissdoerfer/shepherd/blob/master/docs/user/data_format.rst */
// Note: n8 can overflow uint32, 50mA are 16 bit as uA, 26 bit as nA, 34 bit as nA_n8-factor
// TODO: negative residue compensation, new undocumented feature to compensate for noise around 0 - current uint-design cuts away negative part and leads to biased mean()
#define NOISE_ESTIMATE_nA   (2000u)
#define RESIDUE_SIZE_FACTOR (30u)
#define RESIDUE_MAX_nA      (NOISE_ESTIMATE_nA * RESIDUE_SIZE_FACTOR)
uint32_t cal_conv_adc_raw_to_nA(const uint32_t current_raw)
{
	static uint32_t negative_residue_nA = 0;
	const uint32_t I_nA = (uint32_t)(((uint64_t)current_raw * (uint64_t)cal->adc_current_factor_nA_n8) >> 8u);
	// avoid mixing signed and unsigned OPs
	if (cal->adc_current_offset_nA >= 0)
	{
		const uint32_t adc_offset_nA = cal->adc_current_offset_nA;
		return add64(I_nA, adc_offset_nA);
	}
	else
	{
		const uint32_t adc_offset_nA = -cal->adc_current_offset_nA + negative_residue_nA;

		if (I_nA > adc_offset_nA)
		{
			return (I_nA - adc_offset_nA);
		}
		else
		{
			negative_residue_nA = adc_offset_nA - I_nA;
			if (negative_residue_nA > RESIDUE_MAX_nA) negative_residue_nA = RESIDUE_MAX_nA;
			return 0u;
		}
	}
}

// safe conversion - 5 V is 13 bit as mV, 23 bit as uV, 31 bit as uV_n8
uint32_t cal_conv_uV_to_dac_raw(const uint32_t voltage_uV)
{
	uint32_t dac_raw = 0u;
	// return (((uint64_t)(voltage_uV - cal->dac_voltage_offset_uV) * (uint64_t)cal->dac_voltage_inv_factor_uV_n20) >> 20u);
	// avoid mixing signed and unsigned OPs
	if (cal->dac_voltage_offset_uV >= 0)
	{
		const uint32_t dac_offset_uV = cal->dac_voltage_offset_uV;
		if (voltage_uV > dac_offset_uV)	dac_raw = ((uint64_t)(voltage_uV - dac_offset_uV) * (uint64_t)cal->dac_voltage_inv_factor_uV_n20) >> 20u;
		// else dac_raw = 0u;
	}
	else
	{
		const uint32_t dac_offset_uV = -cal->dac_voltage_offset_uV;
		dac_raw = ((uint64_t)(voltage_uV + dac_offset_uV) * (uint64_t)cal->dac_voltage_inv_factor_uV_n20) >> 20u;
	}
	return (dac_raw > 0xFFFFu) ? 0xFFFFu : dac_raw;
}