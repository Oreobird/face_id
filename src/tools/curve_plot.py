import json
import os
import matplotlib.pyplot as plt

class Data_Parser():
    def __init__(self, data_file):
        self.data_file = data_file
        
    def parse_cmc(self):
        with open(self.data_file) as fp:
            data = json.load(fp)
        return data['cmc']
    
    def parse_roc(self):
        with open(self.data_file) as fp:
            data = json.load(fp)
        return data['roc']
    

# Megaface result curve plotter
class MF_Curve_Plotter():
    def __init__(self, data_parser):
        self.parser = data_parser
        
    def cmc(self):
        cmc_data = self.parser.parse_cmc()
        # print(cmc_data)
        plt.plot(cmc_data[0], cmc_data[1])
        plt.show()
        
    def roc(self):
        roc_data = self.parser.parse_roc()
        plt.plot(roc_data[0], roc_data[1])
        plt.show()
    

if __name__ == '__main__':
    cmc_file_path = "E:/ai/aivideo/face_id/cmc_facescrub_megaface_tf-insightface_1000000_1.json"
    data_parser = Data_Parser(cmc_file_path)
    curve_ploter = MF_Curve_Plotter(data_parser)
    curve_ploter.cmc()
    curve_ploter.roc()
    