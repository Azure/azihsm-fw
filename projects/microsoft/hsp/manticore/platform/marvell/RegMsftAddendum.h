// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef REG_MSFT_ADDENDUM_H_
#define REG_MSFT_ADDENDUM_H_


/* These are convenience macros which will help reduce redundant code in source files to
 * program registers for different delivery and completion queue instances for GDMA. */
typedef GdmaGdmaDeliveryQueue0Cfg_t GdmaGdmaDeliveryQueueCfg_t;
typedef GdmaGdmaDeliveryQueue0ProducerIndex_t GdmaGdmaDeliveryQueueProducerIndex_t;
typedef GdmaGdmaDeliveryQueue0ConsumerIndex_t GdmaGdmaDeliveryQueueConsumerIndex_t;
typedef GdmaGdmaCompletionQueue0ProducerIndex_t GdmaGdmaCompletionQueueProducerIndex_t;
typedef GdmaGdmaCompletionQueue0ConsumerIndex_t GdmaGdmaCompletionQueueConsumerIndex_t;
typedef GdmaGdmaCompletionQueue0Cfg_t GdmaGdmaCompletionQueueCfg_t;


#endif // REG_MSFT_ADDENDUM_H_