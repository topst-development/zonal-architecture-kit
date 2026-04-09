#include "enlight_network.h"

// RAW output
typedef struct {
  int dims[4]; // NCHW
  float scale;
  void *data; // size = C * H * W * sizeof(float)
} enlight_output_buffer_t;

typedef struct {
  int num_output;
  enlight_output_buffer_t output[MAX_NUM_OUTPUT];
} enlight_output_buffers_t;

int run_post_process(void *net_inst, void *output_base, int num_input,
                     enlight_output_buffers_t *buffers) {
  enlight_network_t *inst = (enlight_network_t *)net_inst;
  enlight_custom_postproc_t *custom_param =
      (enlight_custom_postproc_t *)inst->post_proc_extension;

  enlight_act_tensor_t *output_tensors[MAX_NUM_OUTPUT];
  int num_output =
      enlight_custom_get_output_tensors(custom_param, output_tensors);

  buffers->num_output = num_output;

  for (int i = 0; i < num_output; i++) {
    enlight_act_tensor_t *tensor = output_tensors[i];
    tensor->base = output_base;

    enlight_output_buffer_t *buffer = &(buffers->output[i]);

    enlight_get_tensor_dimensions(tensor, buffer->dims);
    buffer->scale = tensor->scale;
    buffer->data = &tensor->base[tensor->buf];
  }

  return num_output;
}
