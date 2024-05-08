# Vulkan Glossary

*from https://vkguide.dev/docs/introduction/vulkan_execution/*

- **VkInstance**: The Vulkan context, used to access drivers.
- **VkPhysicalDevice**: A GPU. Used to query physical GPU details, like features, capabilities, memory size, etc.
- **VkDevice**: The “logical” GPU context that you actually execute things on.
- **VkBuffer**: A chunk of GPU visible memory.
- **VkImage**: A texture you can write to and read from.
- **VkPipeline**: Holds the state of the gpu needed to draw. For example: shaders, rasterization options, depth settings.
- **VkRenderPass**: Holds information about the images you are rendering into. All drawing commands have to be done inside a renderpass. Only used in legacy vkguide.
- **VkFrameBuffer**: Holds the target images for a renderpass. Only used in legacy vkguide.
- **VkCommandBuffer**: Encodes GPU commands. All execution that is performed on the GPU itself (not in the driver) has to be encoded in a VkCommandBuffer.
- **VkQueue**: Execution “port” for commands. GPUs will have a set of queues with different properties. Some allow only graphics commands, others only allow memory commands, etc. Command buffers are executed by submitting them into a queue, which will copy the rendering commands onto the GPU for execution.
- **VkDescriptorSet**: Holds the binding information that connects shader inputs to data such as VkBuffer resources and VkImage textures. Think of it as a set of gpu-side pointers that you bind once.
- **VkSwapchainKHR**: Holds the images for the screen. It allows you to render things into a visible window. The KHR suffix shows that it comes from an extension, which in this case is VK_KHR_swapchain.
- **VkSemaphore**: Synchronizes GPU to GPU execution of commands. Used for syncing multiple command buffer submissions one after another.
- **VkFence**: Synchronizes GPU to CPU execution of commands. Used to know if a command buffer has finished being executed on the GPU.
