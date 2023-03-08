import sys
sys.path.append('/software/firmware/pru0-cython-module')
#import numpy as np
from Cvirtual_converter import VirtualConverter
import pytest

class Test_VirtualConverter():
	def __init__(self, access):
		self.access = access
		
	def Test_get_V_intermediate_uV(self):
		return self.access.get_V_intermediate_uV()
		
	def Test_get_P_input_fW(self):
		return self.access.get_P_input_fW() 
		
	def Test_get_P_output_fW(self):
		return self.access.get_P_output_fW() 
		
	def Test_get_I_mid_out_nA(self):
		return self.access.get_I_mid_out_nA() 
	
	def Test_get_input_efficiency_n8(self):
		return self.access.get_input_efficiency_n8(self, 2, 4) 
		
	def Test_get_output_inv_efficiency_n4(self):
		return self.access.get_output_inv_efficiency_n4(self, -4)
		
	def Test_div_uV_n4(self):
		return self.access.div_uV_n4(self, 100000, 2000000)
	
	def Test_Method8(self):
		return self.access.converter_calc_out_power(100000)
	
def test_get_V_intermediate_uV():
	assert accessVC.get_V_intermediate_uV() == 0	
if __name__ == "__main__":    
	# TempList = list(np.random.randint(low = 0, high=9, size=25))

	accessVC = VirtualConverter()
	accessInstanceTest=Test_VirtualConverter(accessVC)

	print(accessInstanceTest.Test_get_V_intermediate_uV())
	print(accessInstanceTest.Test_get_P_input_fW())
	print(accessInstanceTest.Test_get_P_output_fW())
	print(accessInstanceTest.Test_get_I_mid_out_nA())
	print(accessInstanceTest.Test_get_input_efficiency_n8())	
	print(accessInstanceTest.Test_get_output_inv_efficiency_n4())	
	print(accessInstanceTest.Test_div_uV_n4())			
	print(accessInstanceTest.Test_Method8())
	
	
"""
import sys
import numpy as np
from Cvirtual_converter import VirtualConverter

class TestVirtualConverter:
    
    def setup_method(self):
        self.accessVC = VirtualConverter()
        self.accessInstanceTest = TestVirtualConverter(self.accessVC)
        
    def test_get_V_intermediate_uV(self):
        assert self.accessInstanceTest.Test_get_V_intermediate_uV() == self.accessVC.get_V_intermediate_uV()
        
    def test_get_P_input_fW(self):
        assert self.accessInstanceTest.Test_get_P_input_fW() == self.accessVC.get_P_input_fW()
        
    def test_get_P_output_fW(self):
        assert self.accessInstanceTest.Test_get_P_output_fW() == self.accessVC.get_P_output_fW()
        
    def test_get_I_mid_out_nA(self):
        assert self.accessInstanceTest.Test_get_I_mid_out_nA() == self.accessVC.get_I_mid_out_nA()
    
    def test_get_input_efficiency_n8(self):
        assert self.accessInstanceTest.Test_get_input_efficiency_n8() == self.accessVC.get_input_efficiency_n8(2, 4)
        
    def test_get_output_inv_efficiency_n4(self):
        assert self.accessInstanceTest.Test_get_output_inv_efficiency_n4() == self.accessVC.get_output_inv_efficiency_n4(-4)
        
    def test_div_uV_n4(self):
        assert self.accessInstanceTest.Test_div_uV_n4() == self.accessVC.div_uV_n4(100000, 2000000)
        
    def test_Method8(self):
        assert self.accessInstanceTest.Test_Method8() == self.accessVC.converter_calc_out_power(100000)
"""
	
#####################################################################################################################################

"""	This section is back-up for trial, once the functions are fixed and tested it will be removed		"""
			
#####################################################################################################################################	
# sys.path.append("/vishal/shepherd/software/firmware/pru0-cython-module")
# print(sys.path)	
# import pyximport	
#from shepherd.software.virtual_source_config import VirtualSourceConfig

#vscfg = VirtualSourceConfig()
#vconv = VirtualConverter(vscfg.export_for_sysfs())

#x = int(0)
#print(VirtualConverter.get_I_mid_out_nA())
#print(VirtualConverter.converter_calc_inp_power(5, 6))
#print(VirtualConverter.converter_initialize(0))
    