"""
This is onnx builder.
Usage:
   builder = OnnxBuilder()
   input = []
   output = []
   builder.input(input_desc).output(output_desc).nodes(node_desc).edge(edge_desc).build()
"""
import onnx
from onnx import helper


class OnnxType:
    """
    # BFLOAT16 = None # (!) real value is '<TensorProtoDataType.BFLOAT16: 16>'
    # BOOL = None # (!) real value is '<TensorProtoDataType.BOOL: 9>'
    # COMPLEX128 = None # (!) real value is '<TensorProtoDataType.COMPLEX128: 15>'
    # COMPLEX64 = None # (!) real value is '<TensorProtoDataType.COMPLEX64: 14>'
    # DOUBLE = None # (!) real value is '<TensorProtoDataType.DOUBLE: 11>'
    # FLOAT = None # (!) real value is '<TensorProtoDataType.FLOAT: 1>'
    # FLOAT16 = None # (!) real value is '<TensorProtoDataType.FLOAT16: 10>'
    # INT16 = None # (!) real value is '<TensorProtoDataType.INT16: 5>'
    # INT32 = None # (!) real value is '<TensorProtoDataType.INT32: 6>'
    # INT64 = None # (!) real value is '<TensorProtoDataType.INT64: 7>'
    # INT8 = None # (!) real value is '<TensorProtoDataType.INT8: 3>'
    # STRING = None # (!) real value is '<TensorProtoDataType.STRING: 8>'
    # UINT16 = None # (!) real value is '<TensorProtoDataType.UINT16: 4>'
    # UINT32 = None # (!) real value is '<TensorProtoDataType.UINT32: 12>'
    # UINT64 = None # (!) real value is '<TensorProtoDataType.UINT64: 13>'
    # UINT8 = None # (!) real value is '<TensorProtoDataType.UINT8: 2>'
    """
    BFLOAT16 = onnx.TensorProto.BFLOAT16
    FLOAT32 = onnx.TensorProto.FLOAT
    FLOAT16 = onnx.TensorProto.FLOAT16


class OnnxIO:
    def __init__(self, name, shape, data_type=OnnxType.FLOAT32):
        self.name = name
        self.data_type = data_type
        self.shape = shape

    def build(self):
        return helper.make_tensor_info(self.name, self.data_type, self.shape)


class OnnxBuilder:
    def __init__(self):
        self.model = None
        self.inputs = []
        self.outputs = []

    def input(self, inputs: list):
        """
        :param inputs: ['']
        :return:
        """
        return self

    def output(self, outputs: list):
        """
        :return:
        """
        return self

    def node(self, nodes: list):
        return self

    def edge(self, edges: list):
        return self

    def build(self, file_name="DefaultOnnx.onnx", graph_name='DefaultGraph'):
        """
        :param file_name:
        :param graph_name:
        :return:
        """
        # 创建模型。
        model = helper.make_model()

        # 创建输入节点。
        input_node = helper.make_tensor_info("input", onnx.TensorProto.FLOAT, input_shape)

        # 创建输出节点。
        output_node = helper.make_tensor_info("output", onnx.TensorProto.FLOAT, output_shape)

        # 将节点添加到模型中。
        for node in nodes:
            model.add_node(node)

        # 设置输入和输出节点。
        model.set_input(input_node.name, input_node)
        model.set_output(output_node.name, output_node)

        graph = helper.make_graph(nodes, graph_name, self.inputs)
        # 保存模型。
        onnx.save_model(model, "model.onnx")
        return model

    def save_onnx_graph(model, filename):
        """
        保存 ONNX 图。
        参数：
          model：ONNX 图。
          filename：保存的文件名。
        """
        onnx.save_model(model, filename)

    def create_node(self, op_type, name, inputs, outputs, attributes):
        """
        创建 ONNX 节点。

        参数：
          op_type：操作类型。
          name：节点名称。
          inputs：节点的输入。
          outputs：节点的输出。
          attributes：节点的属性。

        返回：
          ONNX 节点。
        """
        node = onnx.helper.make_node(
            op_type=op_type,
            name=name,
            inputs=inputs,
            outputs=outputs,
            attributes=attributes,
        )

        return node


if __name__ == '__main__':
    OnnxBuilder().inputs([OnnxIO("grad", [4096, 1024])])